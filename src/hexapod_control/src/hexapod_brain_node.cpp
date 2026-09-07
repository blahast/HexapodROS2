#include "hexapod_control/hexapod_brain_node.hpp"

HexapodBrainNode::HexapodBrainNode(const rclcpp::NodeOptions & options) 
    : Node("hexapod_brain", options), current_state_(State::INIT), current_gait_id_(0) {
    
    // Reliable QoS for commands and state feedback, best effort for high frequency motion data
    auto reliable_qos = rclcpp::QoS(10).reliable();
    auto best_effort_qos = rclcpp::QoS(10).best_effort();

    // Publishers for buzzer and locomotion commands
    buzzer_pub_ = this->create_publisher<hexapod_custom_msgs::msg::BuzzerCommand>("play_melody", reliable_qos);
    locomotion_cmd_pub_ = this->create_publisher<hexapod_custom_msgs::msg::LocomotionCommand>("locomotion_command", reliable_qos);
    loc_gait_pub_ = this->create_publisher<std_msgs::msg::UInt8>("locomotion/set_gait", reliable_qos);

    // Publishers for motion commands
    loc_vel_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("locomotion/cmd_vel", best_effort_qos);
    loc_pose_pub_ = this->create_publisher<geometry_msgs::msg::Pose>("locomotion/body_pose", best_effort_qos);
    loc_params_pub_ = this->create_publisher<std_msgs::msg::Float32MultiArray>("locomotion/walking_params", best_effort_qos);

    // Subscriptions for teleop inputs and locomotion state feedback
    teleop_event_sub_ = this->create_subscription<hexapod_custom_msgs::msg::TeleopEvent>(
        "teleop_events", reliable_qos, 
        [this](const hexapod_custom_msgs::msg::TeleopEvent::SharedPtr msg) { this->teleopEventCallback(msg); });

    locomotion_status_sub_ = this->create_subscription<hexapod_custom_msgs::msg::LocomotionState>(
        "locomotion_status", reliable_qos, 
        [this](const hexapod_custom_msgs::msg::LocomotionState::SharedPtr msg) { this->locomotionStatusCallback(msg); });

    teleop_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
        "cmd_vel", best_effort_qos, 
        [this](const geometry_msgs::msg::Twist::SharedPtr msg) { this->cmdVelCallback(msg); });

    teleop_pose_sub_ = this->create_subscription<geometry_msgs::msg::Pose>(
        "body_pose", best_effort_qos, 
        [this](const geometry_msgs::msg::Pose::SharedPtr msg) { this->poseCallback(msg); });

    teleop_params_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
        "walking_params", best_effort_qos, 
        [this](const std_msgs::msg::Float32MultiArray::SharedPtr msg) { this->walkingParamsCallback(msg); });

    RCLCPP_INFO(this->get_logger(), "Hexapod brain node initialized.");
}

// Forward velocity commands only when the robot standing or walking
void HexapodBrainNode::cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg) {
    if (current_state_ == State::STANDING || current_state_ == State::WALKING) {
        loc_vel_pub_->publish(*msg);
    } else {
        emergencyStop();
    }
}

// Forward body pose commands only when the robot standing or walking
void HexapodBrainNode::poseCallback(const geometry_msgs::msg::Pose::SharedPtr msg) {
    if (current_state_ == State::STANDING || current_state_ == State::WALKING) {
        loc_pose_pub_->publish(*msg);
    }
}

// Forward walking parameters regardless of state
void HexapodBrainNode::walkingParamsCallback(const std_msgs::msg::Float32MultiArray::SharedPtr msg) {
    loc_params_pub_->publish(*msg);
}

// Handle teleop events (BT connection, stand/sit toggle, gait change, animation trigger)
void HexapodBrainNode::teleopEventCallback(const hexapod_custom_msgs::msg::TeleopEvent::SharedPtr msg) {
    using Evt = hexapod_custom_msgs::msg::TeleopEvent;
    using Buz = hexapod_custom_msgs::msg::BuzzerCommand;
    using LocCmd = hexapod_custom_msgs::msg::LocomotionCommand;

    switch (msg->event_id) {
        case Evt::BT_CONNECTED:
            RCLCPP_INFO(this->get_logger(), "BT_CONNECTED received");
            playBuzzer(Buz::BT_CONNECTED);
            break;

        case Evt::BT_DISCONNECTED:
            RCLCPP_WARN(this->get_logger(), "BT_DISCONNECTED received - stopping all motion");
            playBuzzer(Buz::BT_DISCONNECTED);
            emergencyStop();   // Stop all motion
            break;

        case Evt::TOGGLE_STAND:
            // Toggle between sitting and standing
            if (current_state_ == State::SITTING) {
                RCLCPP_INFO(this->get_logger(), "TOGGLE_STAND: Standing up");
                playBuzzer(Buz::BEEP);
                sendLocomotionCommand(LocCmd::CMD_STAND_UP);
            } else if (current_state_ == State::STANDING) {
                RCLCPP_INFO(this->get_logger(), "TOGGLE_STAND: Sitting down");
                playBuzzer(Buz::BEEP);
                sendLocomotionCommand(LocCmd::CMD_SIT_DOWN);
            } else {
                RCLCPP_WARN(this->get_logger(), "TOGGLE_STAND: REJECTED: robot is in state '%s' (must be SITTING or STANDING)", stateToString(current_state_));
                playBuzzer(Buz::ERROR);
            }
            break;

        case Evt::CHANGE_GAIT:
            // Gait change is allowed when not in an animation or transition
            if (current_state_ == State::SITTING || current_state_ == State::STANDING || current_state_ == State::WALKING) {
                current_gait_id_ = (current_gait_id_ + 1) % 3; // Cycle through 3 gaits
                RCLCPP_INFO(this->get_logger(), "TOGGLE_GAIT: Changing gait to ID %d", current_gait_id_);
                sendGaitCommand(current_gait_id_);
                playBuzzer(Buz::BEEP);
            } else {
                RCLCPP_WARN(this->get_logger(), "TOGGLE_GAIT: REJECTED: robot is in state '%s' (must be STANDING or WALKING)", stateToString(current_state_));
                playBuzzer(Buz::ERROR);
            }
            break;

        case Evt::PLAY_ANIMATION:
            // Animation can only be triggered while standing
            if (current_state_ == State::STANDING) {
                RCLCPP_INFO(this->get_logger(), "TOGGLE_ANIMATION: Play animation ACCEPTED");
                playBuzzer(Buz::BEEP);
                sendLocomotionCommand(LocCmd::CMD_PLAY_ANIMATION);
            } else {
                RCLCPP_WARN(this->get_logger(), "TOGGLE_ANIMATION: REJECTED: robot is in state '%s' (must be STANDING)", stateToString(current_state_));
                playBuzzer(Buz::ERROR);
            }
            break;

        default:
            RCLCPP_WARN(this->get_logger(), "Received unknown event_id: %d", msg->event_id);
            break;
    }
}

// Update internal state based on feedback from the locomotion node
void HexapodBrainNode::locomotionStatusCallback(const hexapod_custom_msgs::msg::LocomotionState::SharedPtr msg) {
    using Loc = hexapod_custom_msgs::msg::LocomotionState;
    State previous_state = current_state_;

    switch(msg->state_id) {
        case Loc::SITTING: current_state_ = State::SITTING; break;
        case Loc::STANDING: current_state_ = State::STANDING; break;
        case Loc::WALKING: current_state_ = State::WALKING; break;
        case Loc::ANIMATING: current_state_ = State::ANIMATING; break;
        default: current_state_ = State::INIT; break;
    }

    if (current_state_ != previous_state) {
        RCLCPP_INFO(this->get_logger(), "State transition: %s -> %s", stateToString(previous_state), stateToString(current_state_));
    }
}

// Helper to publish a buzzer command
void HexapodBrainNode::playBuzzer(uint8_t command_id) {
    hexapod_custom_msgs::msg::BuzzerCommand msg;
    msg.command_id = command_id;
    buzzer_pub_->publish(msg);
}

// Helper to send a locomotion command (stand, sit, animate)
void HexapodBrainNode::sendLocomotionCommand(uint8_t command_id) {
    hexapod_custom_msgs::msg::LocomotionCommand msg;
    msg.command_id = command_id;
    locomotion_cmd_pub_->publish(msg);
}

// Helper to change the gait
void HexapodBrainNode::sendGaitCommand(uint8_t gait_id) {
    std_msgs::msg::UInt8 msg;
    msg.data = gait_id;
    loc_gait_pub_->publish(msg);
}

// Stop all motion
void HexapodBrainNode::emergencyStop() {
    geometry_msgs::msg::Twist stop_msg; // all zeros
    loc_vel_pub_->publish(stop_msg);
}

// Convert enum to readable string
const char* HexapodBrainNode::stateToString(State state) {
    switch (state) {
        case State::INIT: return "INIT";
        case State::SITTING: return "SITTING";
        case State::STANDING: return "STANDING";
        case State::WALKING: return "WALKING";
        case State::ANIMATING: return "ANIMATING";
        default: return "UNKNOWN";
    }
}