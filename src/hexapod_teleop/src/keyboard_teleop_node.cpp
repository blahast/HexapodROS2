#include <unistd.h>
#include <fcntl.h>
#include <iostream>

#include "hexapod_teleop/keyboard_teleop_node.hpp"

KeyboardTeleopNode::KeyboardTeleopNode(const rclcpp::NodeOptions & options) 
    : Node("keyboard_teleop", options) {
    
    this->declare_parameter("loop_rate_hz", DEFAULT_LOOP_RATE_HZ);
    double loop_rate = this->get_parameter("loop_rate_hz").as_double();

    auto best_effort_qos = rclcpp::QoS(10).best_effort();
    auto reliable_qos = rclcpp::QoS(10).reliable();

    velocity_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", best_effort_qos);
    pose_publisher_ = this->create_publisher<geometry_msgs::msg::Pose>("body_pose", best_effort_qos);
    walking_params_publisher_ = this->create_publisher<std_msgs::msg::Float32MultiArray>("walking_params", best_effort_qos);
    event_publisher_ = this->create_publisher<hexapod_custom_msgs::msg::TeleopEvent>("teleop_events", reliable_qos);

    is_running_ = true;
    keyboard_thread_ = std::thread(&KeyboardTeleopNode::keyboardLoop, this);
    
    RCLCPP_INFO(this->get_logger(), "Keyboard teleop started. (%.1f Hz) Make sure THIS terminal is focused.", loop_rate);
}

KeyboardTeleopNode::~KeyboardTeleopNode() {
    is_running_ = false;
    if (keyboard_thread_.joinable()) {
        keyboard_thread_.join();
    }
    restoreTerminal();
}

void KeyboardTeleopNode::restoreTerminal() {
    if (tty_fd_ >= 0) {
        tcsetattr(tty_fd_, TCSANOW, &original_terminal_settings_);
        close(tty_fd_);
        tty_fd_ = -1;
    }
}

void KeyboardTeleopNode::publishTeleopEvent(uint8_t event_id) {
    hexapod_custom_msgs::msg::TeleopEvent msg;
    msg.event_id = event_id;
    event_publisher_->publish(msg);
}

void KeyboardTeleopNode::publishPose() {
    geometry_msgs::msg::Pose pose_msg;
    pose_msg.position.x = 0; pose_msg.position.y = 0; pose_msg.position.z = pos_z_;
    
    tf2::Quaternion q;
    q.setRPY(roll_, pitch_, yaw_);
    pose_msg.orientation.x = q.x(); 
    pose_msg.orientation.y = q.y();
    pose_msg.orientation.z = q.z(); 
    pose_msg.orientation.w = q.w();
    pose_publisher_->publish(pose_msg);
}

void KeyboardTeleopNode::publishWalkingParams() {
    std_msgs::msg::Float32MultiArray params_msg;
    params_msg.data.push_back(gait_speed_);
    params_msg.data.push_back(step_height_);
    walking_params_publisher_->publish(params_msg);
}

void KeyboardTeleopNode::keyboardLoop() {
    tty_fd_ = open("/dev/tty", O_RDONLY | O_NONBLOCK);
    if (tty_fd_ < 0) {
        RCLCPP_ERROR(this->get_logger(), "Cannot open /dev/tty.");
        return;
    }

    tcgetattr(tty_fd_, &original_terminal_settings_);
    struct termios raw = original_terminal_settings_;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(tty_fd_, TCSANOW, &raw);

    last_motion_time_ = std::chrono::steady_clock::now();
    
    double loop_rate = this->get_parameter("loop_rate_hz").as_double();
    auto loop_period = std::chrono::milliseconds(static_cast<int>(1000.0 / loop_rate));
    auto next_loop_time = std::chrono::steady_clock::now();

    while (is_running_) {
        next_loop_time += loop_period;
        
        char c;
        bool motion_key_pressed = false;

        // Read all characters currently in the buffer
        while (read(tty_fd_, &c, 1) > 0) {
            switch (tolower(c)) {
                // Motion
                case 'w': current_vel_x_ = move_speed_; motion_key_pressed = true; break;
                case 's': current_vel_x_ = -move_speed_; motion_key_pressed = true; break;
                case 'a': current_vel_yaw_ = move_speed_; motion_key_pressed = true; break;
                case 'd': current_vel_yaw_ = -move_speed_; motion_key_pressed = true; break;
                case 'q': current_vel_y_ = move_speed_; motion_key_pressed = true; break;
                case 'e': current_vel_y_ = -move_speed_; motion_key_pressed = true; break;
                
                // Pose adjustments
                case 'r': pos_z_ = std::min(1.0f, (float)(pos_z_ + INCREMENT)); break;
                case 'f': pos_z_ = std::max(-1.0f, (float)(pos_z_ - INCREMENT)); break;
                case 'i': pitch_ = std::min(1.0f, (float)(pitch_ + INCREMENT)); break;
                case 'k': pitch_ = std::max(-1.0f, (float)(pitch_ - INCREMENT)); break;
                case 'j': roll_ = std::min(1.0f, (float)(roll_ + INCREMENT)); break;
                case 'l': roll_ = std::max(-1.0f, (float)(roll_ - INCREMENT)); break;
                case 'u': yaw_ = std::min(1.0f, (float)(yaw_ + INCREMENT)); break;
                case 'o': yaw_ = std::max(-1.0f, (float)(yaw_ - INCREMENT)); break;

                // Walking params tweaks
                case '+': gait_speed_ = std::min(1.0f, gait_speed_ + INCREMENT); break;
                case '-': gait_speed_ = std::max(0.0f, gait_speed_ - INCREMENT); break;

                // Events (Publish exactly when pressed)
                case '1': publishTeleopEvent(hexapod_custom_msgs::msg::TeleopEvent::CHANGE_GAIT); break;
                case '2': publishTeleopEvent(hexapod_custom_msgs::msg::TeleopEvent::TOGGLE_STAND); break;
                case '3': publishTeleopEvent(hexapod_custom_msgs::msg::TeleopEvent::PLAY_ANIMATION); break;
            }
        }

        // Key release timeout
        auto now = std::chrono::steady_clock::now();
        if (motion_key_pressed) {
            last_motion_time_ = now;
        } else {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_motion_time_).count();
            if (elapsed > KEY_TIMEOUT_MS) {
                current_vel_x_ = 0.0f;
                current_vel_y_ = 0.0f;
                current_vel_yaw_ = 0.0f;
            }
        }

        // Publish
        geometry_msgs::msg::Twist vel_msg;
        vel_msg.linear.x = current_vel_x_;
        vel_msg.linear.y = current_vel_y_;
        vel_msg.angular.z = current_vel_yaw_;
        
        velocity_publisher_->publish(vel_msg);
        publishWalkingParams();
        publishPose();

        std::this_thread::sleep_until(next_loop_time);
    }
}