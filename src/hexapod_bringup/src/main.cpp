#include <thread>
#include <memory>
#include <string>
#include <pthread.h>
#include <sched.h>

#include <rclcpp/rclcpp.hpp>
#include "hexapod_teleop/bluetooth_teleop_node.hpp"
#include "hexapod_teleop/keyboard_teleop_node.hpp"
#include "hexapod_hardware/buzzer_driver_node.hpp"
#include "hexapod_hardware/servo_driver_node.hpp"
#include "hexapod_hardware/switch_node.hpp"
#include "hexapod_hardware/imu_node.hpp"
#include "hexapod_control/hexapod_brain_node.hpp"
#include "hexapod_control/hexapod_locomotion_node.hpp"

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);

    // Parse --teleop argument
    std::string teleop_mode = "bt";
    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]) == "--teleop" && i + 1 < argc) {
            teleop_mode = argv[i + 1];
        }
    }

    rclcpp::NodeOptions options;
    options.use_intra_process_comms(true);

    // Create nodes
    std::shared_ptr<rclcpp::Node> teleop_node;
    if (teleop_mode == "keyboard") {
        teleop_node = std::make_shared<KeyboardTeleopNode>(options);
    } else {
        teleop_node = std::make_shared<BluetoothTeleopNode>(options);
    }

    auto buzzer_node     = std::make_shared<BuzzerDriverNode>(options);
    auto switch_node     = std::make_shared<SwitchNode>(options);
    auto imu_node        = std::make_shared<ImuNode>(options);
    auto brain_node      = std::make_shared<HexapodBrainNode>(options);

    auto servo_node      = std::make_shared<ServoDriverNode>(options);
    auto locomotion_node = std::make_shared<HexapodLocomotionNode>(options);

    // Two executors: deterministic RT vs. concurrent IO
    rclcpp::executors::SingleThreadedExecutor rt_executor;
    rclcpp::executors::MultiThreadedExecutor io_executor;

    // RT-critical nodes: sequential, deterministic
    rt_executor.add_node(locomotion_node);
    rt_executor.add_node(servo_node);

    // IO nodes: parallel
    io_executor.add_node(buzzer_node);
    io_executor.add_node(switch_node);
    io_executor.add_node(imu_node);
    io_executor.add_node(brain_node);
    io_executor.add_node(teleop_node);

    // RT thread: pin to CPU 3, SCHED_FIFO prio 90
    std::thread rt_thread([&rt_executor]() {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(3, &cpuset);
        if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
            perror("pthread_setaffinity_np (need CAP_SYS_NICE / sudo?)");
        }

        sched_param sch{};
        int policy = 0;
        if (pthread_getschedparam(pthread_self(), &policy, &sch) == 0) {
            sch.sched_priority = 90;
            if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch) != 0) {
                perror("pthread_setschedparam (need CAP_SYS_NICE / sudo?)");
            }
        }

        rt_executor.spin();
    });

    // Main thread runs IO executor
    io_executor.spin();
    rt_thread.join();

    rclcpp::shutdown();
    return 0;
}