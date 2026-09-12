#include <rclcpp/rclcpp.hpp>
#include "hexapod_hardware/switch_node.hpp"

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SwitchNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}