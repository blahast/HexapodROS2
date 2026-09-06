#include <string>
#include <functional>

#include "hexapod_control/hexapod_brain_node.hpp"
#include "hexapod_custom_msgs/msg/teleop_event.hpp"
#include "hexapod_custom_msgs/msg/buzzer_command.hpp"
#include "hexapod_custom_msgs/msg/locomotion_state.hpp"
#include "hexapod_custom_msgs/msg/locomotion_command.hpp"

HexapodBrainNode::HexapodBrainNode(const rclcpp::NodeOptions & options) 
    : Node("hexapod_brain", options), current_state_(State::INIT) {
    auto reliable_qos = rclcpp::QoS(10).reliable();
    auto best_effort_qos = rclcpp::QoS(10).best_effort();

    // Publishers for buzzer commands
    buzzer_pub_ = this->create_publisher<hexapod_custom_msgs::msg::BuzzerCommand>("play_melody", reliable_qos);

    // Publisher for locomotion commands
    locomotion_cmd_pub_ = this->create_publisher<hexapod_custom_msgs::msg::LocomotionCommand>("locomotion_command", reliable_qos);

    // Publishers for locomotion data
    loc_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("locomotion/cmd_vel", best_effort_qos);
    loc_pose_pub_ = this->create_publisher<geometry_msgs::msg::Pose>("locomotion/body_pose", best_effort_qos);
    loc_params_pub_ = this->create_publisher<std_msgs::msg::Float32MultiArray>("locomotion/walking_params", best_effort_qos);

    // Subscribers for teleop events and locomotion state feedback
    teleop_event_sub_ = this->create_subscription<hexapod_custom_msgs::msg::TeleopEvent>(
        "teleop_events", reliable_qos,
        [this](const hexapod_custom_msgs::msg::TeleopEvent::SharedPtr msg) {
            teleopEventCallback(msg);
        });

    // Subscribers for locomotion state feedback
    locomotion_status_sub_ = this->create_subscription<hexapod_custom_msgs::msg::LocomotionState>(
        "locomotion_status", reliable_qos,
        [this](const hexapod_custom_msgs::msg::LocomotionState::SharedPtr msg) {
            locomotionStatusCallback(msg);
        });

    // Subscribers for walking and pose commands from teleop
    teleop_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "cmd_vel", best_effort_qos,
        [this](const geometry_msgs::msg::Twist::SharedPtr msg) {
            cmdVelCallback(msg);
        });

    teleop_pose_sub_ = this->create_subscription<geometry_msgs::msg::Pose>(
        "body_pose", best_effort_qos,
        [this](const geometry_msgs::msg::Pose::SharedPtr msg) {
            poseCallback(msg);
        });

    teleop_params_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
        "walking_params", best_effort_qos,
        [this](const std_msgs::msg::Float32MultiArray::SharedPtr msg) {
            walkingParamsCallback(msg);
        });

    RCLCPP_INFO(this->get_logger(), "Hexapod brain node (State Machine & Multiplexer) initialized.");
}

// --- Continuous data callbacks ---

void HexapodBrainNode::cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg) {
    if (current_state_ == State::STANDING || current_state_ == State::WALKING) {
        loc_vel_pub_->publish(*msg);
    } else {
        emergencyStop();
    }
}

void HexapodBrainNode::poseCallback(const geometry_msgs::msg::Pose::SharedPtr msg) {
    if (current_state_ == State::STANDING || current_state_ == State::WALKING) {
        loc_pose_pub_->publish(*msg);
    }
}

void HexapodBrainNode::walkingParamsCallback(const std_msgs::msg::Float32MultiArray::SharedPtr msg) {
    loc_params_pub_->publish(*msg);
}

// --- Discrete event handling ---

void HexapodBrainNode::teleopEventCallback(const hexapod_custom_msgs::msg::TeleopEvent::SharedPtr msg) {
    using Evt = hexapod_custom_msgs::msg::TeleopEvent;
    using Buz = hexapod_custom_msgs::msg::BuzzerCommand;
    using LocCmd = hexapod_custom_msgs::msg::LocomotionCommand;

    switch (msg->event_id) {
        
        case Evt::BT_CONNECTED:
            playBuzzer(Buz::BT_CONNECTED);
            break;

        case Evt::BT_DISCONNECTED:
            playBuzzer(Buz::BT_DISCONNECTED);
            emergencyStop();   // Force stop on disconnection
            break;

        case Evt::TOGGLE_STAND:
            // Toggle between sitting and standing
            if (current_state_ == State::SITTING) {
                playBuzzer(Buz::BEEP);
                sendLocomotionCommand(LocCmd::CMD_STAND_UP);
            } else if (current_state_ == State::STANDING) {
                playBuzzer(Buz::BEEP);
                sendLocomotionCommand(LocCmd::CMD_SIT_DOWN);
            } else {
                playBuzzer(Buz::ERROR);
            }
            break;

        case Evt::CHANGE_GAIT:
            // Change gait only when sitting or standing
            if (current_state_ == State::SITTING || current_state_ == State::STANDING) {
                sendLocomotionCommand(LocCmd::CMD_CHANGE_GAIT);
                playBuzzer(Buz::BEEP);
            } else {
                playBuzzer(Buz::ERROR);
            }
            break;

        case Evt::PLAY_ANIMATION:
            if (current_state_ == State::STANDING) {
                playBuzzer(Buz::BEEP);
                sendLocomotionCommand(LocCmd::CMD_PLAY_ANIMATION);
            } else {
                playBuzzer(Buz::ERROR);
            }
            break;

        default:
            RCLCPP_WARN(this->get_logger(), "Přijat neznámý event_id: %d", msg->event_id);
            break;
    }
}

void HexapodBrainNode::locomotionStatusCallback(const hexapod_custom_msgs::msg::LocomotionState::SharedPtr msg) {
    using Loc = hexapod_custom_msgs::msg::LocomotionState;
    State previous_state = current_state_;

    // Update internal state based on locomotion feedback
    switch(msg->state_id) {
        case Loc::SITTING: current_state_ = State::SITTING; break;
        case Loc::STANDING: current_state_ = State::STANDING; break;
        case Loc::WALKING: current_state_ = State::WALKING; break;
        case Loc::ANIMATING: current_state_ = State::ANIMATING; break;
        default: current_state_ = State::INIT; break;
    }

    // Log state transitions
    if (current_state_ != previous_state) {
        RCLCPP_INFO(this->get_logger(), "State transition: %s -> %s", 
                    stateToString(previous_state).c_str(), 
                    stateToString(current_state_).c_str());
    }
}

// --- Helpers ---

void HexapodBrainNode::playBuzzer(uint8_t command_id) {
    hexapod_custom_msgs::msg::BuzzerCommand msg;
    msg.command_id = command_id;
    buzzer_pub_->publish(msg);
}

void HexapodBrainNode::sendLocomotionCommand(uint8_t command_id) {
    hexapod_custom_msgs::msg::LocomotionCommand msg;
    msg.command_id = command_id;
    locomotion_cmd_pub_->publish(msg);
}

void HexapodBrainNode::emergencyStop() {
    geometry_msgs::msg::Twist stop_msg;   // all zeros
    loc_vel_pub_->publish(stop_msg);
}

std::string HexapodBrainNode::stateToString(State state) {
    switch (state) {
        case State::INIT: return "INIT";
        case State::SITTING: return "SITTING";
        case State::STANDING: return "STANDING";
        case State::WALKING: return "WALKING";
        case State::ANIMATING: return "ANIMATING";
        default: return "UNKNOWN";
    }
}