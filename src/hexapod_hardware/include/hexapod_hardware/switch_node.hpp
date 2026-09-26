#ifndef HEXAPOD_DRIVER__SWITCH_NODE_HPP_
#define HEXAPOD_DRIVER__SWITCH_NODE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <gpiod.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <array>
#include <cstdint>
#include <algorithm>
#include <chrono>

#include "hexapod_custom_msgs/msg/switch_event.hpp"


// monitors the six leg contact switches on GPIO lines and publishes SwitchEvent messages

class SwitchNode : public rclcpp::Node
{
public:
  static constexpr const char* DEFAULT_GPIO_CHIP_SWITCH = "/dev/gpiochip4";
  static constexpr std::array<int64_t, 6> DEFAULT_LINE_OFFSETS = {26, 16, 25, 24, 23, 22}; // L1 L2 L3 R3 R2 R1
  static constexpr int64_t DEFAULT_DEBOUNCE_MS = 5; // ms

  explicit SwitchNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  ~SwitchNode() override;

private:
  void monitor_edges();

  rclcpp::Publisher<hexapod_custom_msgs::msg::SwitchEvent>::SharedPtr publisher_;

  std::thread monitor_thread_;
  std::atomic<bool> running_;

  std::string chip_name_;
  std::vector<unsigned int> line_offsets_;

  // Software debouncing state
  std::chrono::milliseconds debounce_timeout_;
  std::vector<std::chrono::steady_clock::time_point> last_event_times_;
};

#endif  // HEXAPOD_DRIVER__SWITCH_NODE_HPP_