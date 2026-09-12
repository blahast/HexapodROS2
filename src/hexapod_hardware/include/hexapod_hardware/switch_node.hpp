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

#include "hexapod_custom_msgs/msg/switch_event.hpp"

class SwitchNode : public rclcpp::Node
{
public:
  static constexpr const char* DEFAULT_GPIO_CHIP_SWITCH = "/dev/gpiochip4";
  static constexpr std::array<int64_t, 6> DEFAULT_LINE_OFFSETS = {16, 19, 20, 21, 26, 27};

  explicit SwitchNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
  ~SwitchNode() override;

private:
  void monitor_edges();

  rclcpp::Publisher<hexapod_custom_msgs::msg::SwitchEvent>::SharedPtr publisher_;

  std::thread monitor_thread_;
  std::atomic<bool> running_;

  std::string chip_name_;
  std::vector<unsigned int> line_offsets_;
};

#endif  // HEXAPOD_DRIVER__SWITCH_NODE_HPP_