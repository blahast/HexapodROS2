#include <rclcpp/rclcpp.hpp>
#include "hexapod_hardware/servo_driver_node.hpp"

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ServoDriverNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}