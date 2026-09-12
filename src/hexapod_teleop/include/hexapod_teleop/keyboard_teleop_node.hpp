#ifndef KEYBOARD_TELEOP_NODE_HPP
#define KEYBOARD_TELEOP_NODE_HPP

#include <termios.h>
#include <atomic>
#include <thread>
#include <string>
#include <chrono>

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <hexapod_custom_msgs/msg/teleop_event.hpp>
#include <tf2/LinearMath/Quaternion.h>

/**
 * Reads single keypresses from the terminal and publishes
 *
 * Key bindings:
 *   W/S : forward/backward (linear.x)
 *   A/D : turn left/right (angular.z)
 *   Q/E : strafe left/right (linear.y)
 *   R/F : increase/decrease body height (pos_z)
 *   I/K : pitch up/down
 *   J/L : roll left/right
 *   U/O : yaw left/right
 *   1   : change gait
 *   2   : toggle stand/sit
 *   3   : play animation
 */

class KeyboardTeleopNode : public rclcpp::Node {
public:
    explicit KeyboardTeleopNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~KeyboardTeleopNode() override;

private:
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr velocity_publisher_;
    rclcpp::Publisher<geometry_msgs::msg::Pose>::SharedPtr pose_publisher_;
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr walking_params_publisher_;
    rclcpp::Publisher<hexapod_custom_msgs::msg::TeleopEvent>::SharedPtr event_publisher_;

    std::thread keyboard_thread_;
    std::atomic<bool> is_running_;
    struct termios original_terminal_settings_;
    int tty_fd_ = -1;

    static constexpr double DEFAULT_LOOP_RATE_HZ = 20.0;
    static constexpr int KEY_TIMEOUT_MS = 500;

    static constexpr float INCREMENT = 0.1f;

    // Body pose state
    float pos_z_ = 0.0f;
    float roll_ = 0.0f;
    float pitch_ = 0.0f;
    float yaw_ = 0.0f;

    // Walking parameters (0.0 to 1.0)
    float gait_speed_ = 0.0f;
    float step_height_ = 0.0f;

    // Movement state
    float move_speed_ = 0.8f;
    float current_vel_x_ = 0.0f;
    float current_vel_y_ = 0.0f;
    float current_vel_yaw_ = 0.0f;
    std::chrono::time_point<std::chrono::steady_clock> last_motion_time_;

    void keyboardLoop();
    void restoreTerminal();
    void publishTeleopEvent(uint8_t event_id);
    void publishPose();
    void publishWalkingParams();
};

#endif // KEYBOARD_TELEOP_NODE_HPP