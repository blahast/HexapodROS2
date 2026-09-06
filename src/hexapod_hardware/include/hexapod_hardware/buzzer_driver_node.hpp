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

class BuzzerDriverNode : public rclcpp::Node {
public:
    explicit BuzzerDriverNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~BuzzerDriverNode();

private:
    // Subscriber
    rclcpp::Subscription<hexapod_custom_msgs::msg::BuzzerCommand>::SharedPtr command_subscriber_;

    // Parameters
    int buzzer_pin_;

    // Threading and State
    std::thread play_thread_;
    std::atomic<bool> is_running_;
    std::queue<uint8_t> melody_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    // Helper methods
    void commandCallback(const hexapod_custom_msgs::msg::BuzzerCommand::SharedPtr msg);
    void playLoop();
    void playTone(int freq_hz, int duration_ms);

    // Melodies
    void playBeep();
    void playStartupMelody();
    void playShutdownMelody();
    void playErrorMelody();
    void playMelody();
    void playBtConnectedMelody();
    void playBtDisconnectedMelody();
};

#endif // HEXAPOD_HARDWARE_BUZZER_DRIVER_NODE_HPP