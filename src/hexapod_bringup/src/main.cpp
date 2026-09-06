#include <rclcpp/rclcpp.hpp>
#include "bluetooth_teleop_node/bluetooth_teleop_node.hpp"
#include "hexapod_hardware/buzzer_driver_node.hpp"
#include "hexapod_control/hexapod_brain_node.hpp"

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);

    // Enable Intra-Process Communication (shared memory)
    rclcpp::NodeOptions options;
    options.use_intra_process_comms(true);

    // Create nodes with IPC enabled
    auto teleop_node = std::make_shared<BluetoothTeleopNode>(options);
    auto buzzer_node = std::make_shared<BuzzerDriverNode>(options);
    auto brain_node = std::make_shared<HexapodBrainNode>(options);

    // Multi-threaded executor to run all nodes in one process
    rclcpp::executors::MultiThreadedExecutor executor;
    executor.add_node(teleop_node);
    executor.add_node(buzzer_node);
    executor.add_node(brain_node);

    executor.spin();
    rclcpp::shutdown();
    return 0;
}