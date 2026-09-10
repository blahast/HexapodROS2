#include <rclcpp/rclcpp.hpp>

#include "hexapod_teleop/keyboard_teleop_node.hpp"

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<KeyboardTeleopNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}