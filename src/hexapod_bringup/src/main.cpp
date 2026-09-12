#include <rclcpp/rclcpp.hpp>
#include "hexapod_teleop/bluetooth_teleop_node.hpp"
#include "hexapod_teleop/keyboard_teleop_node.hpp"
#include "hexapod_hardware/buzzer_driver_node.hpp"
#include "hexapod_hardware/servo_driver_node.hpp"
#include "hexapod_hardware/switch_node.hpp"
#include "hexapod_hardware/imu_node.hpp"
#include "hexapod_control/hexapod_brain_node.hpp"
#include "hexapod_control/hexapod_locomotion_node.hpp"

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);

    std::string teleop_mode = "bt";
    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]) == "--teleop" && i + 1 < argc) {
            teleop_mode = argv[i + 1];
        }
    }

        // Enable Intra-Process Communication (shared memory)
    rclcpp::NodeOptions options;
    options.use_intra_process_comms(true);

    // Multi-threaded executor to run all nodes in one process
    rclcpp::executors::MultiThreadedExecutor executor;

    // Create and add the teleop node based on the command-line argument
    std::shared_ptr<rclcpp::Node> teleop_node;

    if (teleop_mode == "keyboard") {
        teleop_node = std::make_shared<KeyboardTeleopNode>(options);
    } else {
        teleop_node = std::make_shared<BluetoothTeleopNode>(options);
    }

    auto buzzer_node = std::make_shared<BuzzerDriverNode>(options);
    auto servo_node = std::make_shared<ServoDriverNode>(options);
    auto switch_node = std::make_shared<SwitchNode>(options);
    auto imu_node = std::make_shared<ImuNode>(options);
    auto brain_node = std::make_shared<HexapodBrainNode>(options);
    auto locomotion_node = std::make_shared<HexapodLocomotionNode>(options);

    executor.add_node(buzzer_node);
    executor.add_node(servo_node);
    executor.add_node(switch_node);
    executor.add_node(imu_node);
    executor.add_node(brain_node);
    executor.add_node(locomotion_node);
    executor.add_node(teleop_node);
    
    executor.spin();
    rclcpp::shutdown();
    return 0;
}