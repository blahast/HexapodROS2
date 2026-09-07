#include <rclcpp/rclcpp.hpp>
#include "hexapod_control/hexapod_brain_node.hpp"

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<HexapodBrainNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}