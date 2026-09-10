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
#include "hexapod_custom_msgs/msg/gait_command.hpp"


// Central decision making node, state machine, translates teleop events into locomotion commands,
class HexapodBrainNode : public rclcpp::Node {
public:
    explicit HexapodBrainNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~HexapodBrainNode() = default;

private:
    enum class State { INIT, SITTING, STANDING, WALKING, ANIMATING };

    State current_state_;          // Last known state from locomotion_status
    uint8_t current_gait_id_;      // Gait index (0..2)

    // Subscriptions for teleop inputs
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr teleop_vel_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Pose>::SharedPtr teleop_pose_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr teleop_params_sub_;
    rclcpp::Subscription<hexapod_custom_msgs::msg::TeleopEvent>::SharedPtr teleop_event_sub_;
    rclcpp::Subscription<hexapod_custom_msgs::msg::LocomotionState>::SharedPtr locomotion_status_sub_;

    // Publishers to the locomotion node and buzzer node
    rclcpp::Publisher<hexapod_custom_msgs::msg::BuzzerCommand>::SharedPtr buzzer_pub_;
    rclcpp::Publisher<hexapod_custom_msgs::msg::LocomotionCommand>::SharedPtr locomotion_cmd_pub_;
    rclcpp::Publisher<hexapod_custom_msgs::msg::GaitCommand>::SharedPtr loc_gait_pub_;
    
    // Publishers for motion commands (forwarded from teleop)
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr loc_vel_pub_;
    rclcpp::Publisher<geometry_msgs::msg::Pose>::SharedPtr loc_pose_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr loc_params_pub_;

    // Callbacks for teleop data
    void cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg);
    void poseCallback(const geometry_msgs::msg::Pose::SharedPtr msg);
    void walkingParamsCallback(const std_msgs::msg::Float32MultiArray::SharedPtr msg);

    // Callbacks for events and state feedback
    void teleopEventCallback(const hexapod_custom_msgs::msg::TeleopEvent::SharedPtr msg);
    void locomotionStatusCallback(const hexapod_custom_msgs::msg::LocomotionState::SharedPtr msg);

    // Helper functions
    void playBuzzer(uint8_t command_id);
    void sendLocomotionCommand(uint8_t command_id);
    void sendGaitCommand(uint8_t gait_id);
    void emergencyStop();
    const char* stateToString(State state);
    const char* gaitToString(uint8_t gait_id);
};

#endif // HEXAPOD_BRAIN_NODE_HPP