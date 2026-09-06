#ifndef HEXAPOD_BRAIN_NODE_HPP
#define HEXAPOD_BRAIN_NODE_HPP

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/pose.hpp>

#include "hexapod_custom_msgs/msg/teleop_event.hpp"
#include "hexapod_custom_msgs/msg/locomotion_state.hpp"
#include "hexapod_custom_msgs/msg/buzzer_command.hpp"
#include "hexapod_custom_msgs/msg/locomotion_command.hpp"

// State machine node that filters teleop data based on robot state
// and forwards commands to locomotion and buzzer subsystems.
class HexapodBrainNode : public rclcpp::Node {
public:
    explicit HexapodBrainNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~HexapodBrainNode() = default;

private:
    // Robot operational states (mirrors locomotion subsystem states)
    enum class State {
        INIT,
        SITTING,
        STANDING,
        WALKING,
        ANIMATING
    };

    State current_state_;

    // Subscribers from teleop node (raw commands & events)
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr teleop_vel_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Pose>::SharedPtr teleop_pose_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr teleop_params_sub_;
    rclcpp::Subscription<hexapod_custom_msgs::msg::TeleopEvent>::SharedPtr teleop_event_sub_;

    // Subscriber for locomotion state feedback
    rclcpp::Subscription<hexapod_custom_msgs::msg::LocomotionState>::SharedPtr locomotion_status_sub_;

    // Publishers for actuation commands
    rclcpp::Publisher<hexapod_custom_msgs::msg::BuzzerCommand>::SharedPtr buzzer_pub_;
    rclcpp::Publisher<hexapod_custom_msgs::msg::LocomotionCommand>::SharedPtr locomotion_cmd_pub_;
    
    // Filtered publishers that forward teleop data only when state allows
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr loc_vel_pub_;
    rclcpp::Publisher<geometry_msgs::msg::Pose>::SharedPtr loc_pose_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr loc_params_pub_;

    // Continuous data callbacks
    void cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg);
    void poseCallback(const geometry_msgs::msg::Pose::SharedPtr msg);
    void walkingParamsCallback(const std_msgs::msg::Float32MultiArray::SharedPtr msg);

    // iscrete event callbacks
    void teleopEventCallback(const hexapod_custom_msgs::msg::TeleopEvent::SharedPtr msg);
    void locomotionStatusCallback(const hexapod_custom_msgs::msg::LocomotionState::SharedPtr msg);

    // Helpers
    void playBuzzer(uint8_t command_id);
    void sendLocomotionCommand(uint8_t command_id);
    void emergencyStop();   // Publish zero velocity to halt motion
    std::string stateToString(State state);
};

#endif // HEXAPOD_BRAIN_NODE_HPP