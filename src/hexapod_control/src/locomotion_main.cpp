#include <rclcpp/rclcpp.hpp>
#include <memory>
#include "hexapod_control/hexapod_locomotion_node.hpp"

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<HexapodLocomotionNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}