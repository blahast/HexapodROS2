#ifndef HEXAPOD_HARDWARE_BUZZER_DRIVER_NODE_HPP
#define HEXAPOD_HARDWARE_BUZZER_DRIVER_NODE_HPP

#include <thread>
#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <condition_variable>

#include <rclcpp/rclcpp.hpp>

#include "hexapod_custom_msgs/msg/buzzer_command.hpp"

// Receives BuzzerCommand messages and plays corresponding melodies on a buzzer

class BuzzerDriverNode : public rclcpp::Node {
public:
    explicit BuzzerDriverNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~BuzzerDriverNode();

private:
    // Subscriber for incoming buzzer commands
    rclcpp::Subscription<hexapod_custom_msgs::msg::BuzzerCommand>::SharedPtr command_subscriber_;

    // GPIO pin number
    int buzzer_pin_;

    // Threading
    std::thread play_thread_;
    std::atomic<bool> is_running_;
    std::queue<uint8_t> melody_queue_;          // queue of commands
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;          // used to wake the worker

    // Callback that pushes commands into the queue
    void commandCallback(const hexapod_custom_msgs::msg::BuzzerCommand::SharedPtr msg);

    // Main loop
    void playLoop();

    // Low-level tone player, plays a frequency for a given duration
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