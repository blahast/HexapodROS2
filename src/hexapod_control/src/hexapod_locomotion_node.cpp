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
    double t2 = t*t, t3 = t2*t, t4 = t2*t2;
    double t_ease = -20.0*t4*t3 + 70.0*t4*t2 - 84.0*t4*t + 35.0*t4;
    return lerp(start, end, t_ease);
}

// 5th order interpolation for swing trajectory
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

HexapodLocomotionNode::HexapodLocomotionNode(const rclcpp::NodeOptions &options) : Node("hexapod_locomotion", options)
{
    this->declare_parameter("loop_rate", DEFAULT_LOOP_RATE_HZ);
    
    // PID parameters declaration
    this->declare_parameter("pid_roll_kp", 0.5);
    this->declare_parameter("pid_roll_ki", 0.0);
    this->declare_parameter("pid_roll_kd", 0.0);
    this->declare_parameter("pid_pitch_kp", 0.5);
    this->declare_parameter("pid_pitch_ki", 0.0);
    this->declare_parameter("pid_pitch_kd", 0.0);

    loop_rate_hz_ = this->get_parameter("loop_rate").as_double();
    loop_period_s_ = 1.0 / loop_rate_hz_;

    // Initialize PID values
    roll_pid_.kp = this->get_parameter("pid_roll_kp").as_double();
    roll_pid_.ki = this->get_parameter("pid_roll_ki").as_double();
    roll_pid_.kd = this->get_parameter("pid_roll_kd").as_double();
    roll_pid_.output_limit = MAX_ROLL * 1.5; // Allow headroom for compensation

    pitch_pid_.kp = this->get_parameter("pid_pitch_kp").as_double();
    pitch_pid_.ki = this->get_parameter("pid_pitch_ki").as_double();
    pitch_pid_.kd = this->get_parameter("pid_pitch_kd").as_double();
    pitch_pid_.output_limit = MAX_PITCH * 1.5; // Allow headroom for compensation

    auto reliable_qos = rclcpp::QoS(10).reliable();
    auto best_effort_qos = rclcpp::QoS(1).best_effort();

    status_pub_ = this->create_publisher<hexapod_custom_msgs::msg::LocomotionState>("locomotion_status", reliable_qos);
    joint_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("joint_states", best_effort_qos);
    hw_angles_pub_ = this->create_publisher<std_msgs::msg::Float32MultiArray>("locomotion/hardware_angles", best_effort_qos);

    cmd_sub_ = this->create_subscription<hexapod_custom_msgs::msg::LocomotionCommand>(
        "locomotion_command", reliable_qos, 
        [this](const hexapod_custom_msgs::msg::LocomotionCommand::SharedPtr msg) { this->cmdCallback(msg); });

    gait_sub_ = this->create_subscription<hexapod_custom_msgs::msg::GaitCommand>(
        "locomotion/set_gait", reliable_qos, 
        [this](const hexapod_custom_msgs::msg::GaitCommand::SharedPtr msg) { this->gaitCallback(msg); });

    vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "locomotion/cmd_vel", best_effort_qos, 
        [this](const geometry_msgs::msg::Twist::SharedPtr msg) { this->velCallback(msg); });

    pose_sub_ = this->create_subscription<geometry_msgs::msg::Pose>(
        "locomotion/body_pose", best_effort_qos, 
        [this](const geometry_msgs::msg::Pose::SharedPtr msg) { this->poseCallback(msg); });

    params_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
        "locomotion/walking_params", best_effort_qos, 
        [this](const std_msgs::msg::Float32MultiArray::SharedPtr msg) { this->paramsCallback(msg); });

    imu_sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
        "/imu/data", best_effort_qos, 
        [this](const sensor_msgs::msg::Imu::SharedPtr msg) { this->imuCallback(msg); });

    // Switch events subscription (reliable_qos matches publisher in switch_node)
    switch_sub_ = this->create_subscription<hexapod_custom_msgs::msg::SwitchEvent>(
        "switch_events", reliable_qos, 
        [this](const hexapod_custom_msgs::msg::SwitchEvent::SharedPtr msg) { this->switchCallback(msg); });

    // Initialise leg geometry and default positions
    for (int leg = 0; leg < NUM_OF_LEGS; ++leg) {
        leg_alpha_[leg] = ALPHA0 + (M_PI * leg / 3.0);
        leg_anchor_[leg] = {BASE_RADIUS * std::cos(leg_alpha_[leg]), BASE_RADIUS * std::sin(leg_alpha_[leg]), 0.0};
        default_leg_pos_[leg] = {DEFAULT_DISTANCE * std::cos(leg_alpha_[leg]), DEFAULT_DISTANCE * std::sin(leg_alpha_[leg]), DEFAULT_HEIGHT};
        leg_pos_[leg] = {INITIAL_DISTANCE * std::cos(leg_alpha_[leg]), INITIAL_DISTANCE * std::sin(leg_alpha_[leg]), INITIAL_HEIGHT};
    }

    // Prealocate messages
    prealloc_js_msg_.name.resize(NUM_OF_LEGS * JOINTS_PER_LEG);
    prealloc_js_msg_.position.resize(NUM_OF_LEGS * JOINTS_PER_LEG);
    prealloc_hw_msg_.data.resize(NUM_OF_LEGS * JOINTS_PER_LEG);

    const std::string prefixes[] = {"coxa", "femur", "tibia"};
    for (int l = 0; l < NUM_OF_LEGS; ++l) {
        for (int j = 0; j < JOINTS_PER_LEG; ++j) {
            int idx = l * 3 + j;
            prealloc_js_msg_.name[idx] = "leg" + std::to_string(l) + "_" + prefixes[j] + "_joint";
        }
    }

    // Start with tripod gait
    current_beta_ = gaits[0].beta;
    for (int i = 0; i < NUM_OF_LEGS; i++) current_phase_offsets_[i] = gaits[0].phaseOffsets[i];

    // ROS timer
    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(static_cast<int>(1000.0 / loop_rate_hz_)),
        [this]() { this->locomotionLoopStep(); }
    );

    inverseKinematics({current_off_x_, current_off_y_, current_off_z_}, {current_roll_, current_pitch_, current_yaw_});
    publishAngles();
    
    RCLCPP_INFO(this->get_logger(), "Locomotion Node initialized. Timer running at %.1f Hz", loop_rate_hz_);
}

void HexapodLocomotionNode::switchCallback(const hexapod_custom_msgs::msg::SwitchEvent::SharedPtr msg) {
    if (msg->leg_index < NUM_OF_LEGS) {
        leg_contact_[msg->leg_index] = msg->pressed; 
    }
}

// IMU Callback - converts quaternion to Euler angles for PID feedback
void HexapodLocomotionNode::imuCallback(const sensor_msgs::msg::Imu::SharedPtr msg) {
    tf2::Quaternion q(
        msg->orientation.x,
        msg->orientation.y,
        msg->orientation.z,
        msg->orientation.w
    );

    // Ignore invalid zero quaternions
    if (q.length2() < 1e-6) {
        return;
    }
    q.normalize();

    // Store initial orientation as the level reference frame
    if (!imu_initialized_) {
        initial_imu_q_ = q;
        imu_initialized_ = true;
        RCLCPP_INFO(this->get_logger(), "IMU reference orientation initialized.");
    }

    // Compute orientation relative to the initial frame
    tf2::Quaternion q_rel = initial_imu_q_.inverse() * q;
    q_rel.normalize();

    tf2::Matrix3x3 m(q_rel);
    double r, p, y;
    m.getRPY(r, p, y);

    measured_roll_ = r;
    measured_pitch_ = p;
}

void HexapodLocomotionNode::gaitCallback(const hexapod_custom_msgs::msg::GaitCommand::SharedPtr msg) {
    if (msg->gait_id >= msg->hexapod_custom_msgs::msg::GaitCommand::NUM_OF_GAITS) { 
        RCLCPP_WARN(this->get_logger(), "Received invalid gait index %d (max %d); ignoring.", msg->gait_id, msg->hexapod_custom_msgs::msg::GaitCommand::NUM_OF_GAITS - 1);
        return;
    }

    target_gait_ = msg->gait_id;
}

void HexapodLocomotionNode::cmdCallback(const hexapod_custom_msgs::msg::LocomotionCommand::SharedPtr msg) {
    pending_command_ = msg->command_id;
}

void HexapodLocomotionNode::velCallback(const geometry_msgs::msg::Twist::SharedPtr msg) {
    target_vx_ = map_norm(msg->linear.x, gaits[target_gait_].max_speed);
    target_vy_ = map_norm(msg->linear.y, gaits[target_gait_].max_speed);
    target_omega_ = map_norm(msg->angular.z, gaits[target_gait_].max_turning_speed);
}

void HexapodLocomotionNode::poseCallback(const geometry_msgs::msg::Pose::SharedPtr msg) {
    
    // Map to physical limits
    target_off_x_ = map_norm(msg->position.x, MAX_X_OFF);
    target_off_y_ = map_norm(msg->position.y, MAX_Y_OFF);
    target_off_z_ = map_norm(msg->position.z, MAX_Z_OFF);
    
    // Convert quaternion to RPY and map to physical limits
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

// Main control loop, called periodically by the timer
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
        cmd = pending_command_;
        pending_command_ = -1;
        
        tvx = target_vx_; tvy = target_vy_; tom = target_omega_;
        tfreq = target_freq_; tsw = target_step_h_;
        tox = target_off_x_; toy = target_off_y_; toz = target_off_z_;
        tr = target_roll_; tp = target_pitch_; ty = target_yaw_;
        tgait = target_gait_;
    }

    // Process animation commands
    using namespace hexapod_custom_msgs::msg;
    if (cmd == LocomotionCommand::CMD_STAND_UP && current_state_ == State::SITTING) {
        buildStandUpAnimation();
    } else if (cmd == LocomotionCommand::CMD_SIT_DOWN && (current_state_ == State::STANDING || current_state_ == State::WALKING)) {
        is_walking_ = false;
        buildSitDownAnimation();
    } else if (cmd == LocomotionCommand::CMD_PLAY_ANIMATION && current_state_ == State::STANDING) {
        buildWaveAnimation();
    }

    // Animation handling
    if (current_state_ == State::ANIMATING) {
        if (current_anim_frame_ < anim_sequence_.size()) {
            const auto& frame = anim_sequence_[current_anim_frame_];
            anim_t_ += loop_period_s_ / std::max(frame.duration_s, 0.001);
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

            current_roll_  = lerp(anim_start_roll_,  frame.body_roll,  t_eased);
            current_pitch_ = lerp(anim_start_pitch_, frame.body_pitch, t_eased);
            current_yaw_   = lerp(anim_start_yaw_,   frame.body_yaw,   t_eased);
            current_off_x_ = lerp(anim_start_off_x_, frame.body_off_x, t_eased);
            current_off_y_ = lerp(anim_start_off_y_, frame.body_off_y, t_eased);
            current_off_z_ = lerp(anim_start_off_z_, frame.body_off_z, t_eased);

            if (frame_done) {
                anim_t_ = 0.0;
                for (int i = 0; i < NUM_OF_LEGS; ++i) anim_start_pos_[i] = leg_pos_[i];
                anim_start_roll_  = current_roll_;
                anim_start_pitch_ = current_pitch_;
                anim_start_yaw_   = current_yaw_;
                anim_start_off_x_ = current_off_x_;
                anim_start_off_y_ = current_off_y_;
                anim_start_off_z_ = current_off_z_;
                current_anim_frame_++;
            }
        } else {
            publishState(state_after_anim_); // Animation finished
        }
        
        double roll_correction = roll_pid_.compute(current_roll_, measured_roll_, loop_period_s_);
        double pitch_correction = pitch_pid_.compute(current_pitch_, measured_pitch_, loop_period_s_);

        double final_roll = current_roll_ + roll_correction;
        double final_pitch = current_pitch_ + pitch_correction;

        inverseKinematics({current_off_x_, current_off_y_, current_off_z_}, {final_roll, final_pitch, current_yaw_});
        publishAngles();
        return; // Skip locomotion
    }

    // Locomotion and walking
    if (current_state_ == State::STANDING || current_state_ == State::WALKING) {
        
        // Exponential smoothing of locomotion parameters
        current_freq_    = smooth_var(current_freq_, tfreq, loop_period_s_, PARAMETER_SMOOTHING_TC, 1e-2);
        current_vx_      = smooth_var(current_vx_, tvx, loop_period_s_, PARAMETER_SMOOTHING_TC, 1e-2);
        current_vy_      = smooth_var(current_vy_, tvy, loop_period_s_, PARAMETER_SMOOTHING_TC, 1e-2);
        current_omega_   = smooth_var(current_omega_, tom, loop_period_s_, PARAMETER_SMOOTHING_TC, 1e-4);
        current_swing_h_ = smooth_var(current_swing_h_, tsw, loop_period_s_, PARAMETER_SMOOTHING_TC, 1e-2);
        
        current_off_x_ = smooth_var(current_off_x_, tox, loop_period_s_, PARAMETER_SMOOTHING_TC, 0.1);
        current_off_y_ = smooth_var(current_off_y_, toy, loop_period_s_, PARAMETER_SMOOTHING_TC, 0.1);
        current_off_z_ = smooth_var(current_off_z_, toz, loop_period_s_, PARAMETER_SMOOTHING_TC, 0.1);
        current_roll_  = smooth_var(current_roll_, tr, loop_period_s_, PARAMETER_SMOOTHING_TC, 1e-4);
        current_pitch_ = smooth_var(current_pitch_, tp, loop_period_s_, PARAMETER_SMOOTHING_TC, 1e-4);
        current_yaw_   = smooth_var(current_yaw_, ty, loop_period_s_, PARAMETER_SMOOTHING_TC, 1e-4);

        // Exponential gait transition smoothing
        current_beta_ = smooth_var(current_beta_, gaits[tgait].beta, loop_period_s_, GAIT_SMOOTHING_TC, 1e-4);
        for (int l = 0; l < NUM_OF_LEGS; ++l) {
            double diff = gaits[tgait].phaseOffsets[l] - current_phase_offsets_[l];

            if (diff > 0.5) diff -= 1.0;
            else if (diff < -0.5) diff += 1.0;
            
            current_phase_offsets_[l] += diff * (1.0 - std::exp(-loop_period_s_ / GAIT_SMOOTHING_TC));
            current_phase_offsets_[l] = wrap01(current_phase_offsets_[l]);
        }

        bool want_to_walk = (std::abs(tvx) > 0.01 || std::abs(tvy) > 0.01 || std::abs(tom) > 0.01);
        
        // Handle walking state transitions
        if (want_to_walk) {
            if (!is_walking_) {
                is_walking_ = true;
                phase_base_ = 0.0;
                for (int l = 0; l < NUM_OF_LEGS; ++l) {
                    swing_state_[l] = LegSwingState{};
                    current_leg_z_[l] = leg_pos_[l].z; // Synchronize Z height
                    leg_early_stance_[l] = false;      // Reset early stance flag
                    leg_late_touchdown_[l] = false;    // Reset late touchdown flag
                    lost_contact_ticks_[l] = 0;        // Reset contact loss hysteresis counter
                    leg_vz_[l] = 0.0;                  // Reset vertical velocity tracker
                }
                publishState(State::WALKING);
            }
            stopping_ = false;
        } else if (is_walking_) {
            stopping_ = true;
        }

        if (is_walking_) {
            phase_base_ = wrap01(phase_base_ + current_freq_ * loop_period_s_);
            double T = (current_freq_ > 1e-6) ? (1.0 / current_freq_) : 1e9;
            int legs_home = 0;

            for (int l = 0; l < NUM_OF_LEGS; ++l) {
                double phase = wrap01(phase_base_ + current_phase_offsets_[l]);
                bool leg_at_home = false;

                if (stopping_) {
                    double dx = leg_pos_[l].x - default_leg_pos_[l].x;
                    double dy = leg_pos_[l].y - default_leg_pos_[l].y;
                    double dz = leg_pos_[l].z - DEFAULT_HEIGHT;
                    double dist = std::hypot(dx, dy);

                    if (dist < 5.0 && std::abs(dz) < 1.0 && 
                        std::abs(current_vx_) < 1.0 && std::abs(current_vy_) < 1.0 && std::abs(current_omega_) < 0.02) 
                    {
                        leg_at_home = true;
                    }
                }

                if (!leg_at_home) {
                    generateStepPoint(l, phase, loop_period_s_, current_beta_, T, 
                                      current_omega_, current_vx_, current_vy_, 
                                      DEFAULT_HEIGHT, current_swing_h_);
                } else {
                    legs_home++;
                }
            }

            // Body heigh adaptation
            double avg_z = 0.0;
            int contact_count = 0;
            for (int l = 0; l < NUM_OF_LEGS; ++l) {
                if (leg_contact_[l]) {
                    avg_z += current_leg_z_[l];
                    contact_count++;
                }
            }

            if (contact_count > 0) {
                avg_z /= contact_count;
                double z_error = DEFAULT_HEIGHT - avg_z;
                
                double target_z_velocity = z_error * 1.5;
                target_z_velocity = std::clamp(target_z_velocity, -Z_ADAPTATION_SPEED, Z_ADAPTATION_SPEED);

                current_z_velocity_ = smooth_var(current_z_velocity_, target_z_velocity, loop_period_s_, 0.1, 0.1);

                double correction = current_z_velocity_ * loop_period_s_;

                for (int l = 0; l < NUM_OF_LEGS; ++l) {
                    current_leg_z_[l] += correction;
                    leg_pos_[l].z += correction;
                    if (swing_state_[l].in_swing) {
                        swing_state_[l].z_lo += correction;
                    }
                }
            }

            // All the legs reached default position
            if (stopping_ && legs_home == NUM_OF_LEGS) {
                is_walking_ = false;
                publishState(State::STANDING);
            }
        }

        double roll_correction = roll_pid_.compute(current_roll_, measured_roll_, loop_period_s_);
        double pitch_correction = pitch_pid_.compute(current_pitch_, measured_pitch_, loop_period_s_);

        double final_roll = current_roll_ + roll_correction;
        double final_pitch = current_pitch_ + pitch_correction;

        inverseKinematics({current_off_x_, current_off_y_, current_off_z_}, {final_roll, final_pitch, current_yaw_});
        publishAngles();
    }
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

// Generate next step point for a given leg with terrain adaptation, hysteresis, and overshoot compensation
void HexapodLocomotionNode::generateStepPoint(int leg, double phase, double dt, double beta, double T, 
                       double omega, double vx, double vy, double h, double swing_h) 
{
    double cur_x = leg_pos_[leg].x;
    double cur_y = leg_pos_[leg].y;
    bool global_stance = (phase < beta);
    bool global_swing = !global_stance;

    auto &sw = swing_state_[leg];
    bool contact = leg_contact_[leg];

    // Phase transition detection
    if (global_swing && !sw.in_swing) {
        sw.in_swing = true; 
        sw.first_tick = true;
        leg_early_stance_[leg] = false;
        leg_late_touchdown_[leg] = false;
        lost_contact_ticks_[leg] = 0;
    } else if (global_stance && sw.in_swing) {
        sw.in_swing = false; 
        sw.first_tick = false;
        sw.swings_done++;
        leg_early_stance_[leg] = false;
        lost_contact_ticks_[leg] = 0;
        
        // Swing finished without ground contact -> enter late touchdown search
        if (!contact) {
            leg_late_touchdown_[leg] = true;
        }
    }

    // Transition from late touchdown to normal stance upon ground contact
    if (leg_late_touchdown_[leg] && contact) {
        leg_late_touchdown_[leg] = false;
        lost_contact_ticks_[leg] = 0;
        
        // Compensate vertical overshoot caused by switch and loop latency
        current_leg_z_[leg] += SEEK_SPEED * LATENCY;
        leg_vz_[leg] = 0.0;
    }

    bool effective_stance = global_stance || leg_early_stance_[leg];

    // Stance phase handling
    if (effective_stance) {
        if (leg_late_touchdown_[leg]) {
            // Move straight down to find the ground while keeping XY constant
            current_leg_z_[leg] -= SEEK_SPEED * dt;
            leg_vz_[leg] = -SEEK_SPEED;
        } else {
            // Normal stance XY movement
            Vec2 p = stance_map_step({cur_x, cur_y}, dt, vx, vy, omega);
            leg_pos_[leg].x = p.x;
            leg_pos_[leg].y = p.y;

            if (!contact) {
                // Increment hysteresis counter when contact is lost during stance
                lost_contact_ticks_[leg]++;
                
                // Start moving downward only after contact is lost for N consecutive ticks
                if (lost_contact_ticks_[leg] >= CONTACT_LOSS_HYSTERESIS_TICKS) {
                    current_leg_z_[leg] -= SEEK_SPEED * dt;
                    leg_vz_[leg] = -SEEK_SPEED;
                } else {
                    leg_vz_[leg] = 0.0;
                }
            } else {
                // Contact regained during stance
                if (lost_contact_ticks_[leg] >= CONTACT_LOSS_HYSTERESIS_TICKS) {
                    // Apply overshoot compensation if the leg was actively descending
                    current_leg_z_[leg] += SEEK_SPEED * LATENCY;
                }
                lost_contact_ticks_[leg] = 0;
                leg_vz_[leg] = 0.0;
            }
        }
        leg_pos_[leg].z = current_leg_z_[leg];
        return;
    }

    // Swing phase handling
    double tau = (phase - beta) / (1.0 - beta);
    if (sw.first_tick) {
        sw.x_lo = cur_x; 
        sw.y_lo = cur_y; 
        sw.z_lo = current_leg_z_[leg]; 
        sw.tau0 = tau;
        double dt_half = 0.5 * T * beta;
        Vec2 TD = stance_map_step({default_leg_pos_[leg].x, default_leg_pos_[leg].y}, -dt_half, vx, vy, omega);
        sw.x_td = TD.x; 
        sw.y_td = TD.y;
        sw.first_tick = false;
        leg_vz_[leg] = 0.0;
    }

    // Early touchdown detection during the descending half of the swing phase
    if (tau > 0.5 && contact) {
        leg_early_stance_[leg] = true;
        lost_contact_ticks_[leg] = 0;

        // Compensate overshoot if the leg was moving downward
        if (leg_vz_[leg] < 0.0) {
            current_leg_z_[leg] += (-leg_vz_[leg]) * LATENCY;
        }
        leg_vz_[leg] = 0.0;

        // Advance XY using stance kinematics for this tick
        Vec2 p = stance_map_step({cur_x, cur_y}, dt, vx, vy, omega);
        leg_pos_[leg].x = p.x;
        leg_pos_[leg].y = p.y;
        leg_pos_[leg].z = current_leg_z_[leg];
        return;
    }

    // Swing trajectory interpolation
    double s_abs = smoothstep5(tau);
    double denom = 1.0 - sw.tau0;
    double tau_rel = (denom > 1e-6) ? (tau - sw.tau0) / denom : 1.0;
    double s_rel = smoothstep5(tau_rel);

    leg_pos_[leg].x = lerp(sw.x_lo, sw.x_td, s_rel);
    leg_pos_[leg].y = lerp(sw.y_lo, sw.y_td, s_rel);

    // Compute new Z height and track vertical velocity for potential early touchdown
    double prev_z = current_leg_z_[leg];
    double expected_z = lerp(sw.z_lo, h, s_rel);
    double new_z = expected_z + 4.0 * swing_h * s_abs * (1.0 - s_abs);

    leg_vz_[leg] = (dt > 1e-6) ? ((new_z - prev_z) / dt) : 0.0;
    leg_pos_[leg].z = new_z;
    current_leg_z_[leg] = new_z;
}

// Rotate point by negative RPY and translate
void HexapodLocomotionNode::rotateNegRPYAndTranslate(const Vector3& p, const Vector3& off_xyz, const Vector3& off_rpy, 
                              double &x_ee, double &y_ee, double &z_ee) 
{
    double dx = p.x - off_xyz.x;
    double dy = p.y - off_xyz.y;
    double dz = p.z - off_xyz.z;

    double cp = std::cos(off_rpy.x), sp = std::sin(off_rpy.x);
    double ct = std::cos(off_rpy.y), st = std::sin(off_rpy.y);
    double cpsi = std::cos(off_rpy.z), spsi = std::sin(off_rpy.z);

    // Rotate by -yaw, -pitch, -roll
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

// Compute joint angles from leg target positions
void HexapodLocomotionNode::inverseKinematics(const Vector3& off_xyz, const Vector3& off_rpy) {
    for (int leg = 0; leg < NUM_OF_LEGS; ++leg) {
        double x_ee, y_ee, z_ee;
        
        rotateNegRPYAndTranslate(leg_pos_[leg], off_xyz, off_rpy, x_ee, y_ee, z_ee);

        double xc = leg_anchor_[leg].x;
        double yc = leg_anchor_[leg].y;
        double delta_x = x_ee - xc;
        double delta_y = y_ee - yc;
        
        // Coxa
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
        } else {
            // Unreachable position, keep previous angles
        }
    }
}

// Publish joint states (for RViz) and hardware angles (for servo drivers)
void HexapodLocomotionNode::publishAngles() {
    prealloc_js_msg_.header.stamp = this->now();
    
    for (int l = 0; l < NUM_OF_LEGS; ++l) {
        for (int j = 0; j < JOINTS_PER_LEG; ++j) {
            // Linear index
            int idx = l * 3 + j;
            
            if (j == 0) {
                prealloc_js_msg_.position[idx] = target_angles_rad_[l][j] * URDF_COXA_SIGN;
            } else if (j == 1) {
                prealloc_js_msg_.position[idx] = target_angles_rad_[l][j] * URDF_FEMUR_SIGN;
            } else {
                prealloc_js_msg_.position[idx] = target_angles_rad_[l][j] * URDF_TIBIA_SIGN;
            }

            double deg = target_angles_rad_[l][j] * 180.0 / M_PI;
            const auto& cfg = servo_config[l][j];
            double hw_ang = deg + cfg.angle_offset;
            if (cfg.inverted) hw_ang = 180.0 - hw_ang;

            // Check servo limits
            if (hw_ang < cfg.min_angle || hw_ang > cfg.max_angle) {
                hw_ang = std::clamp(hw_ang, cfg.min_angle, cfg.max_angle);
            }

            prealloc_hw_msg_.data[idx] = hw_ang;
        }
    }
    
    joint_pub_->publish(prealloc_js_msg_);
    hw_angles_pub_->publish(prealloc_hw_msg_);
}

// Animation handling functions

void HexapodLocomotionNode::startAnimation(const std::vector<AnimKeyframe>& sequence, State next_state) {
    anim_sequence_ = sequence;
    current_anim_frame_ = 0;
    anim_t_ = 0.0;
    state_after_anim_ = next_state;
    for(int i = 0; i < NUM_OF_LEGS; i++) anim_start_pos_[i] = leg_pos_[i];
    
    anim_start_roll_  = current_roll_;
    anim_start_pitch_ = current_pitch_;
    anim_start_yaw_   = current_yaw_;
    anim_start_off_x_ = current_off_x_;
    anim_start_off_y_ = current_off_y_;
    anim_start_off_z_ = current_off_z_;

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
    const double wave_x_distance = 170.0;
    const double wave_y_out = -120.0;
    const double wave_y_in = -80.0;
    const double wave_z_up = 160.0;
    const double wave_z_down = 60.0;
    const double rapid_duration = 0.35;

    std::vector<AnimKeyframe> seq(6);
    
    for (int k=0; k<6; ++k) {
        for (int l=0; l<NUM_OF_LEGS; ++l) seq[k].leg_targets[l] = default_leg_pos_[l];
        seq[k].body_roll = 0.0; seq[k].body_pitch = 0.0;
    }
    
    seq[0].leg_targets[5] = {wave_x_distance, wave_y_out, wave_z_up};
    seq[0].duration_s = 1.5;
    seq[0].ease = true;
    seq[0].body_pitch = -0.20;
    seq[0].body_roll = -0.20;

    seq[1].leg_targets[5] = {wave_x_distance, wave_y_in, wave_z_down};
    seq[1].duration_s = rapid_duration;
    seq[1].ease = false;
    seq[1].body_pitch = -0.20;
    seq[1].body_roll = -0.20;

    seq[2].leg_targets[5] = {wave_x_distance, wave_y_out, wave_z_up};
    seq[2].duration_s = rapid_duration;
    seq[2].ease = false;
    seq[2].body_pitch = -0.20;
    seq[2].body_roll = -0.20;

    seq[3].leg_targets[5] = {wave_x_distance, wave_y_in, wave_z_down};
    seq[3].duration_s = rapid_duration;
    seq[3].ease = false;
    seq[3].body_pitch = -0.20;
    seq[3].body_roll = -0.20;

    seq[4].leg_targets[5] = {wave_x_distance, wave_y_out, wave_z_up};
    seq[4].duration_s = rapid_duration;
    seq[4].ease = false;
    seq[4].body_pitch = -0.20;
    seq[4].body_roll = -0.20;

    seq[5].leg_targets[5] = default_leg_pos_[5];
    seq[5].duration_s = 1.5;
    seq[5].ease = true;
    seq[5].body_pitch = 0.0;
    seq[5].body_roll = 0.0;
    
    startAnimation(seq, State::STANDING);
}