#include <chrono>
#include <memory>
#include <utility>
#include <algorithm>

#include "hexapod_hardware/switch_node.hpp"

SwitchNode::SwitchNode(const rclcpp::NodeOptions & options) : Node("switch_node", options), running_(true)
{
  this->declare_parameter("gpio_chip", DEFAULT_GPIO_CHIP_SWITCH);
  this->declare_parameter("line_offsets", std::vector<int64_t>(DEFAULT_LINE_OFFSETS.begin(), DEFAULT_LINE_OFFSETS.end()));

  chip_name_ = this->get_parameter("gpio_chip").as_string();

  auto offsets_param = this->get_parameter("line_offsets").as_integer_array();
  for (int64_t offset : offsets_param) {
    line_offsets_.push_back(static_cast<unsigned int>(offset));
  }

  publisher_ = this->create_publisher<hexapod_custom_msgs::msg::SwitchEvent>(
    "switch_events", rclcpp::QoS(10).reliable());

  monitor_thread_ = std::thread(&SwitchNode::monitor_edges, this);
}

SwitchNode::~SwitchNode()
{
  running_ = false;
  if (monitor_thread_.joinable()) {
    monitor_thread_.join();
  }
}

void SwitchNode::monitor_edges()
{
  try {
    gpiod::chip chip(chip_name_);
    gpiod::line_bulk lines = chip.get_lines(line_offsets_);

    // Detect press and release
    lines.request({
      this->get_name(),
      gpiod::line_request::EVENT_BOTH_EDGES,
      gpiod::line_request::FLAG_BIAS_PULL_UP
    });

    while (running_ && rclcpp::ok()) {
      auto event_lines = lines.event_wait(std::chrono::milliseconds(100));

      if (event_lines) {
        for (auto & line : event_lines) {
          gpiod::line_event event = line.event_read();

          auto it = std::find(line_offsets_.begin(), line_offsets_.end(), line.offset());
          if (it == line_offsets_.end()) continue;

          hexapod_custom_msgs::msg::SwitchEvent msg;
          msg.leg_index = static_cast<uint8_t>(std::distance(line_offsets_.begin(), it));
          // FALLING_EDGE means pressed
          msg.pressed = (event.event_type == gpiod::line_event::FALLING_EDGE);

          publisher_->publish(msg);
        }
      }
    }
  } catch (const std::exception & e) {
    RCLCPP_FATAL(this->get_logger(), "GPIOD error in switch_node: %s", e.what());
    running_ = false;
    rclcpp::shutdown();
  }
}