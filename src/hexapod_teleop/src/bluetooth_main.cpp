#include <rclcpp/rclcpp.hpp>

#include "hexapod_teleop/bluetooth_teleop_node.hpp"

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<BluetoothTeleopNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}