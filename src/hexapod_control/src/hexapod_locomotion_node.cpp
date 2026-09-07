#include "hexapod_control/hexapod_locomotion_node.hpp"
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

// =============================================================================
// POMOCNÉ FUNKCE PRO MAPOVÁNÍ A VYHLAZENÍ
// =============================================================================

// Namapuje symetrickou normalizovanou hodnotu [-1, 1] do rozmezí [-max_val, max_val]
inline double map_norm(double val, double max_val) {
    return std::clamp(val, -1.0, 1.0) * max_val;
}

// Namapuje asymetrickou normalizovanou hodnotu [-1, 1] do rozmezí [min_val, max_val]
inline double map_range(double val, double min_val, double max_val) {
    double normalized = std::clamp(val, -1.0, 1.0);
    return min_val + (normalized + 1.0) * 0.5 * (max_val - min_val);
}

inline double lerp(double a, double b, double t) { return a + (b - a) * t; }
inline double wrap01(double x) { return x - std::floor(x); }

double smooth_interp_7(double start, double end, double t) {
    t = std::clamp(t, 0.0, 1.0);
    double t2 = t*t; double t3 = t2*t; double t4 = t2*t2;
    double t_ease = -20.0*t4*t3 + 70.0*t4*t2 - 84.0*t4*t + 35.0*t4;
    return lerp(start, end, t_ease);
}

double smoothstep5(double tau) {
    double t2 = tau*tau, t3 = t2*tau;
    return 6.0*t3*t2 - 15.0*t2*t2 + 10.0*t3;
}

inline double smooth_var(double current, double target, double dt, double tc, double deadband) {
    if (std::abs(current - target) < deadband || tc <= 1e-6) return target;
    double alpha = std::exp(-dt / tc);
    return alpha * current + (1.0 - alpha) * target;
}

Vec2 stance_map_step(Vec2 p, double dt, double vx, double vy, double omega) {
    double wdt = omega * dt;
    if (std::abs(wdt) < 1e-6) {
        return {p.x - vx * dt, p.y - vy * dt};
    }
    double s, c, S, C;
    double a = std::abs(wdt);
    if (a < 1e-3) {
        double w2 = wdt * wdt; double w3 = w2 * wdt;
        double w4 = w2 * w2; double w5 = w4 * wdt;
        s = wdt - (w3 / 6.0) + (w5 / 120.0);
        c = 1.0 - 0.5 * w2 + (w4 / 24.0);
        S = dt * (1.0 - (w2 / 6.0) + (w4 / 120.0));
        C = dt * (0.5 * wdt - (w3 / 24.0) + (w5 / 720.0));
    } else {
        s = std::sin(wdt); c = std::cos(wdt);
        S = s / omega; C = (1.0 - c) / omega;
    }
    return {p.x * c - p.y * s - vx * S + vy * C, p.y * c + p.x * s - vy * S - vx * C};
}

// =============================================================================
// IMPLEMENTACE TŘÍDY
// =============================================================================

HexapodLocomotionNode::HexapodLocomotionNode(const rclcpp::NodeOptions &options) 
    : Node("hexapod_locomotion", options)
{
    this->declare_parameter("loop_rate", 50.0);
    loop_rate_hz_ = this->get_parameter("loop_rate").as_double();
    loop_period_s_ = 1.0 / loop_rate_hz_;

    auto reliable_qos = rclcpp::QoS(10).reliable();
    auto best_effort_qos = rclcpp::QoS(10).best_effort();

    status_pub_ = this->create_publisher<hexapod_custom_msgs::msg::LocomotionState>("locomotion_status", reliable_qos);
    joint_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("joint_states", best_effort_qos);
    hw_angles_pub_ = this->create_publisher<std_msgs::msg::Float32MultiArray>("locomotion/hardware_angles", best_effort_qos);

    cmd_sub_ = this->create_subscription<hexapod_custom_msgs::msg::LocomotionCommand>(
        "locomotion_command", reliable_qos, std::bind(&HexapodLocomotionNode::cmdCallback, this, std::placeholders::_1));
    vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "locomotion/cmd_vel", best_effort_qos, std::bind(&HexapodLocomotionNode::velCallback, this, std::placeholders::_1));
    pose_sub_ = this->create_subscription<geometry_msgs::msg::Pose>(
        "locomotion/body_pose", best_effort_qos, std::bind(&HexapodLocomotionNode::poseCallback, this, std::placeholders::_1));
    params_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
        "locomotion/walking_params", best_effort_qos, std::bind(&HexapodLocomotionNode::paramsCallback, this, std::placeholders::_1));

    for (int leg = 0; leg < NUM_OF_LEGS; ++leg) {
        leg_alpha_[leg] = ALPHA0 + (M_PI * leg / 3.0);
        leg_anchor_[leg] = {BASE_RADIUS * std::cos(leg_alpha_[leg]), BASE_RADIUS * std::sin(leg_alpha_[leg]), 0.0};
        default_leg_pos_[leg] = {DEFAULT_DISTANCE * std::cos(leg_alpha_[leg]), DEFAULT_DISTANCE * std::sin(leg_alpha_[leg]), DEFAULT_HEIGHT};
        leg_pos_[leg].target = {INITIAL_DISTANCE * std::cos(leg_alpha_[leg]), INITIAL_DISTANCE * std::sin(leg_alpha_[leg]), INITIAL_HEIGHT};
        stopping_swings_[leg] = 0;
    }

    run_thread_ = true;
    locomotion_thread_ = std::thread(&HexapodLocomotionNode::locomotionLoop, this);
    
    RCLCPP_INFO(this->get_logger(), "Locomotion Node initialized. Loop rate: %.1f Hz", loop_rate_hz_);
}

HexapodLocomotionNode::~HexapodLocomotionNode() {
    run_thread_ = false;
    if (locomotion_thread_.joinable()) {
        locomotion_thread_.join();
    }
}

void HexapodLocomotionNode::cmdCallback(const hexapod_custom_msgs::msg::LocomotionCommand::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    pending_command_ = msg->command_id;
}

void HexapodLocomotionNode::velCallback(const geometry_msgs::msg::Twist::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    target_vx_ = map_norm(msg->linear.x, gaits[current_gait_].max_speed);
    target_vy_ = map_norm(msg->linear.y, gaits[current_gait_].max_speed);
    target_omega_ = map_norm(msg->angular.z, gaits[current_gait_].max_turning_speed);
}

void HexapodLocomotionNode::poseCallback(const geometry_msgs::msg::Pose::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    // Zpráva nese hodnoty [-1, 1], mapujeme je do fyzických limitů
    target_off_x_ = map_norm(msg->position.x, MAX_X_OFF);
    target_off_y_ = map_norm(msg->position.y, MAX_Y_OFF);
    target_off_z_ = map_norm(msg->position.z, MAX_Z_OFF);
    
    // Převod kvaternionu z ROS zprávy na RPY a rovnou oříznutí mapováním
    tf2::Quaternion q(msg->orientation.x, msg->orientation.y, msg->orientation.z, msg->orientation.w);
    tf2::Matrix3x3 m(q);
    double r, p, y;
    m.getRPY(r, p, y);

    target_roll_ = map_norm(r, MAX_ROLL);
    target_pitch_ = map_norm(p, MAX_PITCH);
    target_yaw_ = map_norm(y, MAX_YAW);
}

void HexapodLocomotionNode::paramsCallback(const std_msgs::msg::Float32MultiArray::SharedPtr msg) {
    if (msg->data.size() >= 2) {
        std::lock_guard<std::mutex> lock(data_mutex_);
        target_freq_ = map_range(msg->data[0], MIN_FREQ, MAX_FREQ);
        target_step_h_ = map_range(msg->data[1], MIN_STEP_H, MAX_STEP_H);
    }
}

void HexapodLocomotionNode::publishState(State s) {
    using namespace hexapod_custom_msgs::msg;
    LocomotionState msg;
    switch(s) {
        case State::SITTING: msg.state_id = LocomotionState::SITTING; break;
        case State::STANDING: msg.state_id = LocomotionState::STANDING; break;
        case State::WALKING: msg.state_id = LocomotionState::WALKING; break;
        case State::ANIMATING: msg.state_id = LocomotionState::ANIMATING; break;
        default: break;
    }
    status_pub_->publish(msg);
    current_state_ = s;
}

void HexapodLocomotionNode::locomotionLoop() {
    rclcpp::Rate rate(loop_rate_hz_);
    
    double freq = MIN_FREQ, omega = 0.0, vx = 0.0, vy = 0.0, swing_h = MIN_STEP_H;
    double phase_base = 0.0;
    bool is_walking = false;
    bool stopping = false;
    bool was_stopping = false;
    
    publishState(State::SITTING);

    while (rclcpp::ok() && run_thread_) {
        int cmd = -1;
        {
            std::lock_guard<std::mutex> lock(data_mutex_);
            cmd = pending_command_;
            pending_command_ = -1;
        }

        using namespace hexapod_custom_msgs::msg;
        if (cmd == LocomotionCommand::CMD_STAND_UP && current_state_ == State::SITTING) {
            publishState(State::ANIMATING);
            interpolateOffsetsToZero(0.5); 
            standUpAnimation();
            publishState(State::STANDING);
        } else if (cmd == LocomotionCommand::CMD_SIT_DOWN && (current_state_ == State::STANDING || current_state_ == State::WALKING)) {
            is_walking = false;
            publishState(State::ANIMATING);
            interpolateOffsetsToZero(1.0);
            sitDownAnimation();
            publishState(State::SITTING);
        } else if (cmd == LocomotionCommand::CMD_CHANGE_GAIT) {
            std::lock_guard<std::mutex> lock(data_mutex_);
            current_gait_ = (current_gait_ + 1) % 3;
        } else if (cmd == LocomotionCommand::CMD_PLAY_ANIMATION && current_state_ == State::STANDING) {
            publishState(State::ANIMATING);
            interpolateOffsetsToZero(1.0);
            waveAnimation();
            publishState(State::STANDING);
        }

        if (current_state_ == State::STANDING || current_state_ == State::WALKING) {
            double tvx, tvy, tom, tfreq, tsw;
            double tox, toy, toz, tr, tp, ty;
            int tgait;
            
            {
                std::lock_guard<std::mutex> lock(data_mutex_);
                tvx = target_vx_; tvy = target_vy_; tom = target_omega_;
                tfreq = target_freq_; tsw = target_step_h_;
                tox = target_off_x_; toy = target_off_y_; toz = target_off_z_;
                tr = target_roll_; tp = target_pitch_; ty = target_yaw_;
                tgait = current_gait_;
            }

            freq    = smooth_var(freq, tfreq, loop_period_s_, 0.3, 1e-2);
            vx      = smooth_var(vx, tvx, loop_period_s_, 0.3, 1e-2);
            vy      = smooth_var(vy, tvy, loop_period_s_, 0.3, 1e-2);
            omega   = smooth_var(omega, tom, loop_period_s_, 0.3, 1e-4);
            swing_h = smooth_var(swing_h, tsw, loop_period_s_, 0.3, 1e-2);
            
            current_off_x_ = smooth_var(current_off_x_, tox, loop_period_s_, 0.3, 0.1);
            current_off_y_ = smooth_var(current_off_y_, toy, loop_period_s_, 0.3, 0.1);
            current_off_z_ = smooth_var(current_off_z_, toz, loop_period_s_, 0.3, 0.1);
            
            // Plynulé vyhlazování RPY (stejně jako v původním C kódu)
            current_roll_ = smooth_var(current_roll_, tr, loop_period_s_, 0.3, 1e-4);
            current_pitch_ = smooth_var(current_pitch_, tp, loop_period_s_, 0.3, 1e-4);
            current_yaw_ = smooth_var(current_yaw_, ty, loop_period_s_, 0.3, 1e-4);

            bool want_to_walk = (std::abs(tvx) > 0.01 || std::abs(tvy) > 0.01 || std::abs(tom) > 0.01);
            
            if (want_to_walk && !is_walking) {
                is_walking = true;
                stopping = false;
                phase_base = 0.0;
                for (int l = 0; l < NUM_OF_LEGS; ++l) {
                    stopping_swings_[l] = 0;
                    swing_state_[l] = LegSwingState{};
                }
                publishState(State::WALKING);
            } else if (!want_to_walk && is_walking) {
                stopping = true;
            }

            if (was_stopping && !stopping) {
                for (int l = 0; l < NUM_OF_LEGS; ++l) stopping_swings_[l] = 0;
            }

            if (is_walking) {
                phase_base = wrap01(phase_base + freq * loop_period_s_);
                double beta = gaits[tgait].beta;
                double T = (freq > 1e-6) ? (1.0 / freq) : 1e9;
                int stopping_legs = 0;

                for (int l = 0; l < NUM_OF_LEGS; ++l) {
                    double phase = wrap01(phase_base + gaits[tgait].phaseOffsets[l]);
                    if (!stopping || stopping_swings_[l] < 2) {
                        generateStepPoint(l, phase, loop_period_s_, beta, T, omega, vx, vy, DEFAULT_HEIGHT, swing_h, stopping);
                    }
                    if (stopping && stopping_swings_[l] >= 2) stopping_legs++;
                }

                if (stopping && stopping_legs == NUM_OF_LEGS) {
                    is_walking = false;
                    publishState(State::STANDING);
                }
            } 
            // Odstraněn else blok: Nohy zůstanou přesně tam, kde plynule dostouply díky 2 stopping steps
            
            was_stopping = stopping;

            if (current_state_ != State::ANIMATING) {
                inverseKinematics(
                    {current_off_x_, current_off_y_, current_off_z_},
                    {current_roll_, current_pitch_, current_yaw_}
                );
                publishAngles();
            }
        }
        rate.sleep();
    }
}

void HexapodLocomotionNode::generateStepPoint(int leg, double phase, double dt, double beta, double T, 
                       double omega, double vx, double vy, double h, double swing_h, bool stopping) 
{
    double cur_x = leg_pos_[leg].target.x;
    double cur_y = leg_pos_[leg].target.y;
    bool now_stance = (phase < beta);
    bool now_swing = !now_stance;

    auto &sw = swing_state_[leg];

    if (now_swing && !sw.in_swing) {
        sw.in_swing = true; sw.first_tick = true;
    } else if (now_stance && sw.in_swing) {
        sw.in_swing = false; sw.first_tick = false;
        sw.swings_done++;
        if (stopping) { stopping_swings_[leg]++; return; }
    }

    if (now_stance) {
        Vec2 p = stance_map_step({cur_x, cur_y}, dt, vx, vy, omega);
        leg_pos_[leg].target.x = p.x;
        leg_pos_[leg].target.y = p.y;
        leg_pos_[leg].target.z = h;
        return;
    }

    double tau = (phase - beta) / (1.0 - beta);
    if (sw.first_tick) {
        sw.x_lo = cur_x; sw.y_lo = cur_y; sw.tau0 = tau;
        double dt_half = 0.5 * T * beta;
        Vec2 TD = stance_map_step({default_leg_pos_[leg].x, default_leg_pos_[leg].y}, -dt_half, vx, vy, omega);
        sw.x_td = TD.x; sw.y_td = TD.y;
        sw.first_tick = false;
    }

    double denom = 1.0 - sw.tau0;
    double tau_rel = (denom > 1e-6) ? (tau - sw.tau0) / denom : 1.0;
    
    double s_rel = smoothstep5(tau_rel);
    double s_abs = smoothstep5(tau);
    
    leg_pos_[leg].target.x = lerp(sw.x_lo, sw.x_td, s_rel);
    leg_pos_[leg].target.y = lerp(sw.y_lo, sw.y_td, s_rel);
    leg_pos_[leg].target.z = h + 4.0 * swing_h * s_abs * (1.0 - s_abs);
}

void HexapodLocomotionNode::rotateNegRPYAndTranslate(const Vector3& p, const Vector3& off_xyz, const Vector3& off_rpy, 
                              double &x_ee, double &y_ee, double &z_ee) 
{
    double dx = p.x - off_xyz.x;
    double dy = p.y - off_xyz.y;
    double dz = p.z - off_xyz.z;

    double cp = std::cos(off_rpy.x), sp = std::sin(off_rpy.x);
    double ct = std::cos(off_rpy.y), st = std::sin(off_rpy.y);
    double cpsi = std::cos(off_rpy.z), spsi = std::sin(off_rpy.z);

    double x1 = dx*cpsi + dy*spsi;
    double y1 = -dx*spsi + dy*cpsi;
    double z1 = dz;

    double x2 = x1*ct - z1*st;
    double y2 = y1;
    double z2 = x1*st + z1*ct;

    x_ee = x2;
    y_ee = y2*cp + z2*sp;
    z_ee = y2*sp - z2*cp;
}

void HexapodLocomotionNode::inverseKinematics(const Vector3& off_xyz, const Vector3& off_rpy) {
    for (int leg = 0; leg < NUM_OF_LEGS; ++leg) {
        double x_ee, y_ee, z_ee;
        
        rotateNegRPYAndTranslate(leg_pos_[leg].target, off_xyz, off_rpy, x_ee, y_ee, z_ee);

        double xc = leg_anchor_[leg].x;
        double yc = leg_anchor_[leg].y;
        double delta_x = x_ee - xc;
        double delta_y = y_ee - yc;
        
        double th_coxa = std::atan2(delta_y, delta_x) - leg_alpha_[leg];
        th_coxa = std::remainder(th_coxa, 2.0 * M_PI);

        double rho = std::hypot(delta_x, delta_y) - L_COXA;
        double R = std::hypot(rho, z_ee);

        if (R >= std::abs(L_FEMUR - L_TIBIA) && R <= (L_FEMUR + L_TIBIA) && rho >= 0.0) {
            double cos_gamma = std::clamp((L_FEMUR*L_FEMUR + L_TIBIA*L_TIBIA - R*R) / (2.0*L_FEMUR*L_TIBIA), -1.0, 1.0);
            double th_tibia = M_PI - std::acos(cos_gamma) - TIBIA_ANGLE_RAD;

            double cos_alpha = std::clamp((L_FEMUR*L_FEMUR + R*R - L_TIBIA*L_TIBIA) / (2.0*L_FEMUR * std::max(R, 1e-9)), -1.0, 1.0);
            double th_femur = std::acos(cos_alpha) - std::atan2(z_ee, rho);

            target_angles_rad_[leg][0] = th_coxa;
            target_angles_rad_[leg][1] = th_femur;
            target_angles_rad_[leg][2] = th_tibia;
        }
    }
}

void HexapodLocomotionNode::publishAngles() {
    sensor_msgs::msg::JointState js_msg;
    js_msg.header.stamp = this->now();
    
    std_msgs::msg::Float32MultiArray hw_msg;
    hw_msg.data.reserve(18);

    for (int l = 0; l < NUM_OF_LEGS; ++l) {
        const std::string prefixes[] = {"coxa", "femur", "tibia"};
        for (int j = 0; j < JOINTS_PER_LEG; ++j) {
            js_msg.name.push_back("leg" + std::to_string(l) + "_" + prefixes[j] + "_joint");
            
            // Násobení URDF konstantami srovná pohled v RVizu dle běžné ROS konvence.
            if (j == 0) js_msg.position.push_back(target_angles_rad_[l][j] * URDF_COXA_SIGN);
            else if (j == 1) js_msg.position.push_back(target_angles_rad_[l][j] * URDF_FEMUR_SIGN);
            else js_msg.position.push_back(target_angles_rad_[l][j] * URDF_TIBIA_SIGN);
            
            // Hardwarové úhly se zachovávají plně identické jako v C
            double deg = target_angles_rad_[l][j] * 180.0 / M_PI;
            const auto& cfg = servo_config[l][j];
            double hw_ang = deg + cfg.angle_offset;
            if (cfg.inverted) hw_ang = 180.0 - hw_ang;
            hw_ang = std::clamp(hw_ang, cfg.min_angle, cfg.max_angle);
            
            hw_msg.data.push_back(hw_ang);
        }
    }
    joint_pub_->publish(js_msg);
    hw_angles_pub_->publish(hw_msg);
}

void HexapodLocomotionNode::interpolateOffsetsToZero(double duration) {
    if (duration <= 0.0) return;
    
    double start_x = current_off_x_, start_y = current_off_y_, start_z = current_off_z_;
    double start_r = current_roll_, start_p = current_pitch_, start_yaw = current_yaw_;
    
    int steps = duration / loop_period_s_;
    rclcpp::Rate rate(loop_rate_hz_);
    
    for (int i = 0; i <= steps && rclcpp::ok(); ++i) {
        double t = smooth_interp_7(0.0, 1.0, (double)i / steps);
        
        current_off_x_ = lerp(start_x, 0.0, t);
        current_off_y_ = lerp(start_y, 0.0, t);
        current_off_z_ = lerp(start_z, 0.0, t);
        
        current_roll_ = lerp(start_r, 0.0, t);
        current_pitch_ = lerp(start_p, 0.0, t);
        current_yaw_ = lerp(start_yaw, 0.0, t);
        
        inverseKinematics(
            {current_off_x_, current_off_y_, current_off_z_}, 
            {current_roll_, current_pitch_, current_yaw_}
        );
        publishAngles();
        rate.sleep();
    }
}

void HexapodLocomotionNode::interpolateLegs(const Vector3 end_targets[NUM_OF_LEGS], double duration) {
    Vector3 start[NUM_OF_LEGS];
    for (int l=0; l<NUM_OF_LEGS; ++l) start[l] = leg_pos_[l].target;

    int steps = duration / loop_period_s_;
    rclcpp::Rate rate(loop_rate_hz_);
    
    for (int i = 0; i <= steps && rclcpp::ok(); ++i) {
        double t = (double)i / steps;
        for (int l=0; l<NUM_OF_LEGS; ++l) {
            leg_pos_[l].target.x = smooth_interp_7(start[l].x, end_targets[l].x, t);
            leg_pos_[l].target.y = smooth_interp_7(start[l].y, end_targets[l].y, t);
            leg_pos_[l].target.z = smooth_interp_7(start[l].z, end_targets[l].z, t);
        }
        
        inverseKinematics({0,0,0}, {0,0,0}); 
        publishAngles();
        rate.sleep();
    }
}

void HexapodLocomotionNode::standUpAnimation() {
    Vector3 ground[NUM_OF_LEGS], defaults[NUM_OF_LEGS];
    for (int l=0; l<NUM_OF_LEGS; ++l) {
        ground[l] = {default_leg_pos_[l].x, default_leg_pos_[l].y, GROUND_HEIGHT};
        defaults[l] = default_leg_pos_[l];
    }
    interpolateLegs(ground, 1.5);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    interpolateLegs(defaults, 1.5);
}

void HexapodLocomotionNode::sitDownAnimation() {
    Vector3 ground[NUM_OF_LEGS], initials[NUM_OF_LEGS];
    for (int l=0; l<NUM_OF_LEGS; ++l) {
        ground[l] = {leg_pos_[l].target.x, leg_pos_[l].target.y, GROUND_HEIGHT};
        double a = leg_alpha_[l];
        initials[l] = {INITIAL_DISTANCE*std::cos(a), INITIAL_DISTANCE*std::sin(a), INITIAL_HEIGHT};
    }
    interpolateLegs(ground, 1.5);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    interpolateLegs(initials, 1.5);
}

void HexapodLocomotionNode::waveAnimation() {
    rclcpp::Rate rate(loop_rate_hz_);
    
    Vector3 targets[NUM_OF_LEGS];
    for (int l=0; l<NUM_OF_LEGS; ++l) targets[l] = leg_pos_[l].target;
    targets[5] = {160.0, -90.0, 80.0};
    interpolateLegs(targets, 1.5);
    
    double start_tibia = target_angles_rad_[5][2];
    double wave_tibia = start_tibia - (50.0 * M_PI / 180.0);
    
    for (int w = 0; w < 2; ++w) {
        int steps = 0.25 / loop_period_s_;
        for(int i=0; i<=steps && rclcpp::ok(); ++i) {
            target_angles_rad_[5][2] = smooth_interp_7(start_tibia, wave_tibia, (double)i/steps);
            publishAngles();
            rate.sleep();
        }
        for(int i=0; i<=steps && rclcpp::ok(); ++i) {
            target_angles_rad_[5][2] = smooth_interp_7(wave_tibia, start_tibia, (double)i/steps);
            publishAngles();
            rate.sleep();
        }
    }
    
    targets[5] = default_leg_pos_[5];
    interpolateLegs(targets, 1.5);
}