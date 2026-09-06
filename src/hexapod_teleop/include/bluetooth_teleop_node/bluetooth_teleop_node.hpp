#ifndef BLUETOOTH_TELEOP_NODE_HPP
#define BLUETOOTH_TELEOP_NODE_HPP

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <std_msgs/msg/string.hpp>
#include <tf2/LinearMath/Quaternion.h>

#include <termios.h>
#include <atomic>
#include <string>
#include <thread>

#include "hexapod_custom_msgs/msg/teleop_event.hpp"

class BluetoothTeleopNode : public rclcpp::Node {
public:
    explicit BluetoothTeleopNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~BluetoothTeleopNode();

private:
    // Publishers
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr velocity_publisher_;
    rclcpp::Publisher<geometry_msgs::msg::Pose>::SharedPtr pose_publisher_;
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr walking_params_publisher_;
    rclcpp::Publisher<hexapod_custom_msgs::msg::TeleopEvent>::SharedPtr event_publisher_;

    // Parameters
    std::string port_name_;
    int baud_rate_;
    float joystick_center_value_;
    float joystick_deadzone_;
    float potentiometer_denominator_;

    // State variables
    bool is_crab_walk_active_ = false;
    bool is_rpy_offset_active_ = false;
    uint8_t previous_button_state_ = 0;

    // Thread and atomic variables
    std::thread serial_thread_;
    std::atomic<bool> is_running_;
    std::atomic<int> serial_file_descriptor_;

    // Helper methods
    speed_t getBaudRateMacro(int baud);
    void configureSerialRawMode(struct termios* tty_config);
    int openSerialPort(const char* device_path, int baud_rate);
    uint8_t calculateChecksum(const uint8_t* buffer, size_t length);
    float normalizeJoystick(uint16_t raw_value, bool invert = false);
    float normalizePotentiometer(uint16_t raw_value);
    void processTelemetryPacket(const uint8_t* payload);
    void publishTeleopEvent(uint8_t event_id);
    void serialReadLoop();
};

#endif