#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

#include "hexapod_control/hexapod_locomotion_node.hpp"

// Map [-1,1] to [-max_val, max_val]
inline double map_norm(double val, double max_val) {
    return std::clamp(val, -1.0, 1.0) * max_val;
}

// Map [-1,1] to [min_val, max_val]
inline double map_range(double val, double min_val, double max_val) {
    double normalized = std::clamp(val, -1.0, 1.0);
    return min_val + (normalized + 1.0) * 0.5 * (max_val - min_val);
}

inline double lerp(double a, double b, double t) {return a + (b - a) * t; }
inline double wrap01(double x) { return x - std::floor(x); }

// 7th order smooth interpolation
double smooth_interp_7(double start, double end, double t) {
    t = std::clamp(t, 0.0, 1.0);
    double t2 = t*t; double t3 = t2*t; double t4 = t2*t2;
    double t_ease = -20.0*t4*t3 + 70.0*t4*t2 - 84.0*t4*t + 35.0*t4;
    return lerp(start, end, t_ease);
}

// 5th order smoothstep for swing trajectory
double smoothstep5(double tau) {
    double t2 = tau*tau, t3 = t2*tau;
    return 6.0*t3*t2 - 15.0*t2*t2 + 10.0*t3;
}

// Exponential smoothing with deadband
inline double smooth_var(double current, double target, double dt, double tc, double deadband) {
    if (std::abs(current - target) < deadband || tc <= 1e-6) return target;
    double alpha = std::exp(-dt / tc);
    return alpha * current + (1.0 - alpha) * target;
}

// Update leg position in stance phase
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
        "locomotion_command", reliable_qos, 
        [this](const hexapod_custom_msgs::msg::LocomotionCommand::SharedPtr msg) { this->cmdCallback(msg); });

    gait_sub_ = this->create_subscription<std_msgs::msg::UInt8>(
        "locomotion/set_gait", reliable_qos, 
        [this](const std_msgs::msg::UInt8::SharedPtr msg) { this->gaitCallback(msg); });

    vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "locomotion/cmd_vel", best_effort_qos, 
        [this](const geometry_msgs::msg::Twist::SharedPtr msg) { this->velCallback(msg); });

    pose_sub_ = this->create_subscription<geometry_msgs::msg::Pose>(
        "locomotion/body_pose", best_effort_qos, 
        [this](const geometry_msgs::msg::Pose::SharedPtr msg) { this->poseCallback(msg); });

    params_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
        "locomotion/walking_params", best_effort_qos, 
        [this](const std_msgs::msg::Float32MultiArray::SharedPtr msg) { this->paramsCallback(msg); });

    // Initialise leg geometry and default positions
    for (int leg = 0; leg < NUM_OF_LEGS; ++leg) {
        leg_alpha_[leg] = ALPHA0 + (M_PI * leg / 3.0);
        leg_anchor_[leg] = {BASE_RADIUS * std::cos(leg_alpha_[leg]), BASE_RADIUS * std::sin(leg_alpha_[leg]), 0.0};
        default_leg_pos_[leg] = {DEFAULT_DISTANCE * std::cos(leg_alpha_[leg]), DEFAULT_DISTANCE * std::sin(leg_alpha_[leg]), DEFAULT_HEIGHT};
        leg_pos_[leg] = {INITIAL_DISTANCE * std::cos(leg_alpha_[leg]), INITIAL_DISTANCE * std::sin(leg_alpha_[leg]), INITIAL_HEIGHT};
        stopping_swings_[leg] = 0;
    }

    // Start with tripod gait
    current_beta_ = gaits[0].beta;
    for (int i = 0; i < NUM_OF_LEGS; i++) current_phase_offsets_[i] = gaits[0].phaseOffsets[i];

    // Use ROS timer instead of a separate thread
    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(static_cast<int>(1000.0 / loop_rate_hz_)),
        std::bind(&HexapodLocomotionNode::locomotionLoopStep, this)
    );
    
    RCLCPP_INFO(this->get_logger(), "Locomotion Node initialized. Timer running at %.1f Hz", loop_rate_hz_);
}

void HexapodLocomotionNode::gaitCallback(const std_msgs::msg::UInt8::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    target_gait_ = msg->data % 3;
}

void HexapodLocomotionNode::cmdCallback(const hexapod_custom_msgs::msg::LocomotionCommand::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    pending_command_ = msg->command_id;
}

void HexapodLocomotionNode::velCallback(const geometry_msgs::msg::Twist::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    target_vx_ = map_norm(msg->linear.x, gaits[target_gait_].max_speed);
    target_vy_ = map_norm(msg->linear.y, gaits[target_gait_].max_speed);
    target_omega_ = map_norm(msg->angular.z, gaits[target_gait_].max_turning_speed);
}

void HexapodLocomotionNode::poseCallback(const geometry_msgs::msg::Pose::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    // Pose message carries [-1,1] values; map to physical limits
    target_off_x_ = map_norm(msg->position.x, MAX_X_OFF);
    target_off_y_ = map_norm(msg->position.y, MAX_Y_OFF);
    target_off_z_ = map_norm(msg->position.z, MAX_Z_OFF);
    
    // Convert quaternion to RPY and clamp via mapping
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

// Main control loop – called periodically by the timer
void HexapodLocomotionNode::locomotionLoopStep() {
    static bool first_run = true;
    if (first_run) {
        publishState(State::SITTING);
        first_run = false;
    }
    
    int cmd = -1;
    double tvx, tvy, tom, tfreq, tsw;
    double tox, toy, toz, tr, tp, ty;
    int tgait;
    
    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        cmd = pending_command_;
        pending_command_ = -1;
        
        tvx = target_vx_; tvy = target_vy_; tom = target_omega_;
        tfreq = target_freq_; tsw = target_step_h_;
        tox = target_off_x_; toy = target_off_y_; toz = target_off_z_;
        tr = target_roll_; tp = target_pitch_; ty = target_yaw_;
        tgait = target_gait_;
    }

    // --- Process locomotion commands (start animations) ---
    using namespace hexapod_custom_msgs::msg;
    if (cmd == LocomotionCommand::CMD_STAND_UP && current_state_ == State::SITTING) {
        buildStandUpAnimation();
    } else if (cmd == LocomotionCommand::CMD_SIT_DOWN && (current_state_ == State::STANDING || current_state_ == State::WALKING)) {
        is_walking_ = false;
        buildSitDownAnimation();
    } else if (cmd == LocomotionCommand::CMD_PLAY_ANIMATION && current_state_ == State::STANDING) {
        buildWaveAnimation();
    }

    // --- Active animation handling (non‑blocking state machine) ---
    if (current_state_ == State::ANIMATING) {
        // Smooth body offsets to zero while animating legs
        current_off_x_ = smooth_var(current_off_x_, 0.0, loop_period_s_, 0.2, 1e-3);
        current_off_y_ = smooth_var(current_off_y_, 0.0, loop_period_s_, 0.2, 1e-3);
        current_off_z_ = smooth_var(current_off_z_, 0.0, loop_period_s_, 0.2, 1e-3);
        current_roll_ = smooth_var(current_roll_, 0.0, loop_period_s_, 0.2, 1e-4);
        current_pitch_ = smooth_var(current_pitch_, 0.0, loop_period_s_, 0.2, 1e-4);
        current_yaw_ = smooth_var(current_yaw_, 0.0, loop_period_s_, 0.2, 1e-4);

        if (current_anim_frame_ < anim_sequence_.size()) {
            const auto& frame = anim_sequence_[current_anim_frame_];
            anim_t_ += loop_period_s_ / std::max(frame.duration_s, 0.01);
            
            bool frame_done = false;
            if (anim_t_ >= 1.0) {
                anim_t_ = 1.0;
                frame_done = true;
            }

            double t_eased = frame.ease ? smooth_interp_7(0.0, 1.0, anim_t_) : anim_t_;
            for (int i = 0; i < NUM_OF_LEGS; ++i) {
                leg_pos_[i].x = lerp(anim_start_pos_[i].x, frame.leg_targets[i].x, t_eased);
                leg_pos_[i].y = lerp(anim_start_pos_[i].y, frame.leg_targets[i].y, t_eased);
                leg_pos_[i].z = lerp(anim_start_pos_[i].z, frame.leg_targets[i].z, t_eased);
            }

            if (frame_done) {
                anim_t_ = 0.0;
                for (int i = 0; i < NUM_OF_LEGS; ++i) anim_start_pos_[i] = leg_pos_[i];
                current_anim_frame_++;
            }
        } else {
            publishState(state_after_anim_); // Animation finished
        }
        
        inverseKinematics({current_off_x_, current_off_y_, current_off_z_}, {current_roll_, current_pitch_, current_yaw_});
        publishAngles();
        return; // Skip locomotion
    }

    // --- Locomotion and walking ---
    if (current_state_ == State::STANDING || current_state_ == State::WALKING) {
        
        // Exponential smoothing of locomotion parameters
        current_freq_    = smooth_var(current_freq_, tfreq, loop_period_s_, 0.3, 1e-2);
        current_vx_      = smooth_var(current_vx_, tvx, loop_period_s_, 0.3, 1e-2);
        current_vy_      = smooth_var(current_vy_, tvy, loop_period_s_, 0.3, 1e-2);
        current_omega_   = smooth_var(current_omega_, tom, loop_period_s_, 0.3, 1e-4);
        current_swing_h_ = smooth_var(current_swing_h_, tsw, loop_period_s_, 0.3, 1e-2);
        
        current_off_x_ = smooth_var(current_off_x_, tox, loop_period_s_, 0.3, 0.1);
        current_off_y_ = smooth_var(current_off_y_, toy, loop_period_s_, 0.3, 0.1);
        current_off_z_ = smooth_var(current_off_z_, toz, loop_period_s_, 0.3, 0.1);
        current_roll_  = smooth_var(current_roll_, tr, loop_period_s_, 0.3, 1e-4);
        current_pitch_ = smooth_var(current_pitch_, tp, loop_period_s_, 0.3, 1e-4);
        current_yaw_   = smooth_var(current_yaw_, ty, loop_period_s_, 0.3, 1e-4);

        // --- Exponential gait transition smoothing (during walking) ---
        double tc_gait = 0.5; // smoothing time constant
        current_beta_ = smooth_var(current_beta_, gaits[tgait].beta, loop_period_s_, tc_gait, 1e-4);
        for (int l = 0; l < NUM_OF_LEGS; ++l) {
            double diff = gaits[tgait].phaseOffsets[l] - current_phase_offsets_[l];
            // Correct for shortest circular path [0,1]
            if (diff > 0.5) diff -= 1.0;
            else if (diff < -0.5) diff += 1.0;
            
            current_phase_offsets_[l] += diff * (1.0 - std::exp(-loop_period_s_ / tc_gait));
            current_phase_offsets_[l] = wrap01(current_phase_offsets_[l]);
        }
        // ----------------------------------------------------

        bool want_to_walk = (std::abs(tvx) > 0.01 || std::abs(tvy) > 0.01 || std::abs(tom) > 0.01);
        
        if (want_to_walk && !is_walking_) {
            is_walking_ = true;
            stopping_ = false;
            phase_base_ = 0.0;
            for (int l = 0; l < NUM_OF_LEGS; ++l) {
                stopping_swings_[l] = 0;
                swing_state_[l] = LegSwingState{};
            }
            publishState(State::WALKING);
        } else if (!want_to_walk && is_walking_) {
            stopping_ = true;
        }

        if (was_stopping_ && !stopping_) {
            for (int l = 0; l < NUM_OF_LEGS; ++l) stopping_swings_[l] = 0;
        }

        if (is_walking_) {
            phase_base_ = wrap01(phase_base_ + current_freq_ * loop_period_s_);
            double T = (current_freq_ > 1e-6) ? (1.0 / current_freq_) : 1e9;
            int stopping_legs = 0;

            for (int l = 0; l < NUM_OF_LEGS; ++l) {
                double phase = wrap01(phase_base_ + current_phase_offsets_[l]);
                if (!stopping_ || stopping_swings_[l] < 2) {
                    generateStepPoint(l, phase, loop_period_s_, current_beta_, T, 
                                      current_omega_, current_vx_, current_vy_, 
                                      DEFAULT_HEIGHT, current_swing_h_, stopping_);
                }
                if (stopping_ && stopping_swings_[l] >= 2) stopping_legs++;
            }

            if (stopping_ && stopping_legs == NUM_OF_LEGS) {
                is_walking_ = false;
                publishState(State::STANDING);
            }
        } 
        
        was_stopping_ = stopping_;
        
        inverseKinematics({current_off_x_, current_off_y_, current_off_z_}, {current_roll_, current_pitch_, current_yaw_});
        publishAngles();
    }
}

// Generate one step point for a given leg (based on phase and gait parameters)
void HexapodLocomotionNode::generateStepPoint(int leg, double phase, double dt, double beta, double T, 
                       double omega, double vx, double vy, double h, double swing_h, bool stopping) 
{
    double cur_x = leg_pos_[leg].x;
    double cur_y = leg_pos_[leg].y;
    bool now_stance = (phase < beta);
    bool now_swing = !now_stance;

    auto &sw = swing_state_[leg];

    // Detect transitions
    if (now_swing && !sw.in_swing) {
        sw.in_swing = true; sw.first_tick = true;
    } else if (now_stance && sw.in_swing) {
        sw.in_swing = false; sw.first_tick = false;
        sw.swings_done++;
        if (stopping) { stopping_swings_[leg]++; return; }
    }

    if (now_stance) {
        // Stance: follow body motion
        Vec2 p = stance_map_step({cur_x, cur_y}, dt, vx, vy, omega);
        leg_pos_[leg].x = p.x;
        leg_pos_[leg].y = p.y;
        leg_pos_[leg].z = h;
        return;
    }

    // Swing phase
    double tau = (phase - beta) / (1.0 - beta);
    if (sw.first_tick) {
        // Capture lift‑off position and pre‑compute touch‑down position
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
    
    // Interpolate horizontal movement and add vertical arc
    leg_pos_[leg].x = lerp(sw.x_lo, sw.x_td, s_rel);
    leg_pos_[leg].y = lerp(sw.y_lo, sw.y_td, s_rel);
    leg_pos_[leg].z = h + 4.0 * swing_h * s_abs * (1.0 - s_abs);
}

// Rotate point by negative RPY and translate (inverse of body transformation)
void HexapodLocomotionNode::rotateNegRPYAndTranslate(const Vector3& p, const Vector3& off_xyz, const Vector3& off_rpy, 
                              double &x_ee, double &y_ee, double &z_ee) 
{
    double dx = p.x - off_xyz.x;
    double dy = p.y - off_xyz.y;
    double dz = p.z - off_xyz.z;

    double cp = std::cos(off_rpy.x), sp = std::sin(off_rpy.x);
    double ct = std::cos(off_rpy.y), st = std::sin(off_rpy.y);
    double cpsi = std::cos(off_rpy.z), spsi = std::sin(off_rpy.z);

    // Rotate by -yaw, -pitch, -roll (inverse of body orientation)
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

// Compute joint angles from leg target positions using inverse kinematics
void HexapodLocomotionNode::inverseKinematics(const Vector3& off_xyz, const Vector3& off_rpy) {
    for (int leg = 0; leg < NUM_OF_LEGS; ++leg) {
        double x_ee, y_ee, z_ee;
        
        rotateNegRPYAndTranslate(leg_pos_[leg], off_xyz, off_rpy, x_ee, y_ee, z_ee);

        double xc = leg_anchor_[leg].x;
        double yc = leg_anchor_[leg].y;
        double delta_x = x_ee - xc;
        double delta_y = y_ee - yc;
        
        // Coxa angle
        double th_coxa = std::atan2(delta_y, delta_x) - leg_alpha_[leg];
        th_coxa = std::remainder(th_coxa, 2.0 * M_PI);

        double rho = std::hypot(delta_x, delta_y) - L_COXA;
        double R = std::hypot(rho, z_ee);

        // Check reachability
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

// Publish joint states (for RViz) and hardware angles (for servo drivers)
void HexapodLocomotionNode::publishAngles() {
    sensor_msgs::msg::JointState js_msg;
    js_msg.header.stamp = this->now();
    
    std_msgs::msg::Float32MultiArray hw_msg;
    hw_msg.data.reserve(18);

    for (int l = 0; l < NUM_OF_LEGS; ++l) {
        const std::string prefixes[] = {"coxa", "femur", "tibia"};
        for (int j = 0; j < JOINTS_PER_LEG; ++j) {
            js_msg.name.push_back("leg" + std::to_string(l) + "_" + prefixes[j] + "_joint");
            
            // Multiply by URDF signs to align RViz visualisation with ROS convention
            if (j == 0) js_msg.position.push_back(target_angles_rad_[l][j] * URDF_COXA_SIGN);
            else if (j == 1) js_msg.position.push_back(target_angles_rad_[l][j] * URDF_FEMUR_SIGN);
            else js_msg.position.push_back(target_angles_rad_[l][j] * URDF_TIBIA_SIGN);
            
            // Hardware angles are absolute and include offset/inversion as per servo config
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

// --- Non‑blocking animation system ---

void HexapodLocomotionNode::startAnimation(const std::vector<AnimKeyframe>& sequence, State next_state) {
    anim_sequence_ = sequence;
    current_anim_frame_ = 0;
    anim_t_ = 0.0;
    state_after_anim_ = next_state;
    for(int i = 0; i < NUM_OF_LEGS; i++) anim_start_pos_[i] = leg_pos_[i];
    publishState(State::ANIMATING);
}

void HexapodLocomotionNode::buildStandUpAnimation() {
    std::vector<AnimKeyframe> seq(2);
    for (int l=0; l<NUM_OF_LEGS; ++l) {
        seq[0].leg_targets[l] = {default_leg_pos_[l].x, default_leg_pos_[l].y, GROUND_HEIGHT};
        seq[1].leg_targets[l] = default_leg_pos_[l];
    }
    seq[0].duration_s = 1.5; seq[0].ease = true;
    seq[1].duration_s = 1.5; seq[1].ease = true;
    startAnimation(seq, State::STANDING);
}

void HexapodLocomotionNode::buildSitDownAnimation() {
    std::vector<AnimKeyframe> seq(2);
    for (int l=0; l<NUM_OF_LEGS; ++l) {
        seq[0].leg_targets[l] = {leg_pos_[l].x, leg_pos_[l].y, GROUND_HEIGHT};
        double a = leg_alpha_[l];
        seq[1].leg_targets[l] = {INITIAL_DISTANCE*std::cos(a), INITIAL_DISTANCE*std::sin(a), INITIAL_HEIGHT};
    }
    seq[0].duration_s = 1.5; seq[0].ease = true;
    seq[1].duration_s = 1.5; seq[1].ease = true;
    startAnimation(seq, State::SITTING);
}

void HexapodLocomotionNode::buildWaveAnimation() {
    // Wave animation: move leg 5 in a waving pattern using Cartesian targets (IK handles joint limits)
    std::vector<AnimKeyframe> seq(6);
    
    for (int k=0; k<6; ++k) {
        for (int l=0; l<NUM_OF_LEGS; ++l) seq[k].leg_targets[l] = default_leg_pos_[l];
    }
    
    // Leg index 5 (last leg) performs the wave
    seq[0].leg_targets[5] = {160.0, -90.0, 80.0};  seq[0].duration_s = 1.5;  seq[0].ease = true;   // Lift
    seq[1].leg_targets[5] = {160.0, -120.0, 80.0}; seq[1].duration_s = 0.25; seq[1].ease = false;  // Swing left
    seq[2].leg_targets[5] = {160.0, -60.0, 80.0};  seq[2].duration_s = 0.25; seq[2].ease = false;  // Swing right
    seq[3].leg_targets[5] = {160.0, -120.0, 80.0}; seq[3].duration_s = 0.25; seq[3].ease = false;  // Swing left
    seq[4].leg_targets[5] = {160.0, -60.0, 80.0};  seq[4].duration_s = 0.25; seq[4].ease = false;  // Swing right
    seq[5].leg_targets[5] = default_leg_pos_[5];   seq[5].duration_s = 1.5;  seq[5].ease = true;   // Lower
    
    startAnimation(seq, State::STANDING);
}