#include <rclcpp/rclcpp.hpp>
#include "hexapod_hardware/buzzer_driver_node.hpp"

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<BuzzerDriverNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}