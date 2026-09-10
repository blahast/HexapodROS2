#ifndef HEXAPOD_HARDWARE_BUZZER_DRIVER_NODE_HPP
#define HEXAPOD_HARDWARE_BUZZER_DRIVER_NODE_HPP

#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <condition_variable>
#include <memory>
#include <chrono>

#include <rclcpp/rclcpp.hpp>
#include <gpiod.hpp>

#include "hexapod_custom_msgs/msg/buzzer_command.hpp"

constexpr const char* DEFAULT_GPIO_CHIP_BUZZER = "gpiochip4";
constexpr int DEFAULT_BUZZER_PIN = 17;

class BuzzerDriverNode : public rclcpp::Node {
public:
    explicit BuzzerDriverNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~BuzzerDriverNode();

private:
    // Subscriber for incoming buzzer commands
    rclcpp::Subscription<hexapod_custom_msgs::msg::BuzzerCommand>::SharedPtr command_subscriber_;

    // GPIO variables
    std::string buzzer_chip_name_;
    int buzzer_pin_;
    std::unique_ptr<gpiod::chip> gpio_chip_;
    gpiod::line buzzer_line_;

    // Threading
    std::thread play_thread_;
    std::atomic<bool> is_running_;
    std::queue<uint8_t> melody_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    void commandCallback(const hexapod_custom_msgs::msg::BuzzerCommand::SharedPtr msg);
    void playLoop();
    void playTone(int freq_hz, int duration_ms);

    // Predefined melody functions
    void playBeep();
    void playStartupMelody();
    void playShutdownMelody();
    void playErrorMelody();
    void playMelody();
    void playBtConnectedMelody();
    void playBtDisconnectedMelody();
};

#endif // HEXAPOD_HARDWARE_BUZZER_DRIVER_NODE_HPP