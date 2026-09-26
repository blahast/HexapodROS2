#include <chrono>
#include <memory>
#include <thread>
#include <utility>
#include <algorithm>

#include "hexapod_hardware/switch_node.hpp"

SwitchNode::SwitchNode(const rclcpp::NodeOptions & options)
: Node("switch_node", options), running_(true)
{
  this->declare_parameter("gpio_chip", DEFAULT_GPIO_CHIP_SWITCH);
  this->declare_parameter("line_offsets", std::vector<int64_t>(DEFAULT_LINE_OFFSETS.begin(), DEFAULT_LINE_OFFSETS.end()));
  this->declare_parameter("debounce_ms", DEFAULT_DEBOUNCE_MS);

  chip_name_ = this->get_parameter("gpio_chip").as_string();

  auto offsets_param = this->get_parameter("line_offsets").as_integer_array();
  for (int64_t offset : offsets_param) {
    line_offsets_.push_back(static_cast<unsigned int>(offset));
  }

  auto debounce_ms = this->get_parameter("debounce_ms").as_int();
  debounce_timeout_ = std::chrono::milliseconds(debounce_ms);

  // Initialise last event timestamps
  auto time_in_past = std::chrono::steady_clock::now() - std::chrono::hours(1);
  last_event_times_.assign(line_offsets_.size(), time_in_past);

  publisher_ = this->create_publisher<hexapod_custom_msgs::msg::SwitchEvent>("switch_events", rclcpp::QoS(10).reliable());

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

    // Request both edges with an internal pull‑up
    lines.request({this->get_name(),
                   gpiod::line_request::EVENT_BOTH_EDGES,
                   gpiod::line_request::FLAG_BIAS_PULL_UP});

    const size_t N = line_offsets_.size();

    // Last published state: -1 = unknown, 0 = pressed, 1 = released
    std::vector<int>  last_published(N, -1);

    // Is this line waiting for the debounce timeout to settle
    std::vector<bool> pending(N, false);

    // Timestamp of the most recent edge received for each line
    std::vector<std::chrono::steady_clock::time_point> last_edge_time(
        N, std::chrono::steady_clock::now() - std::chrono::hours(1));

    // Confirmation reads guard against glitches on the shared ground line
    constexpr int CONFIRM_READS = 1;
    constexpr auto CONFIRM_DELAY = std::chrono::milliseconds(1);

    while (running_ && rclcpp::ok()) {
      // 5 ms tick
      auto event_lines = lines.event_wait(std::chrono::milliseconds(1));
      auto now = std::chrono::steady_clock::now();

      // Record new edges, update the timestamp
      if (event_lines) {
        for (auto & line : event_lines) {
          gpiod::line_event event = line.event_read();

          auto it = std::find(line_offsets_.begin(), line_offsets_.end(), line.offset());
          if (it == line_offsets_.end()) {
            continue;
          }
          size_t idx = static_cast<size_t>(std::distance(line_offsets_.begin(), it));

          // Push the settling deadline forward on every new edge
          last_edge_time[idx] = now;
          pending[idx] = true;
        }
      }

      // Check which lines have settled
      for (size_t i = 0; i < N; ++i) {
        if (!pending[i]) {
          continue;
        }
        if (now - last_edge_time[i] < debounce_timeout_) {
          continue;
        }

        // rRepeated reads to reject random glitches.
        int value = -1;
        bool stable = true;

        for (int k = 0; k < CONFIRM_READS; ++k) {
          int current = lines.get(i).get_value();

          if (k == 0) {
            value = current;
          } else if (current != value) {
            stable = false;
            break;
          }

          if (k < CONFIRM_READS - 1) {
            std::this_thread::sleep_for(CONFIRM_DELAY);
          }
        }

        if (!stable) {
          last_edge_time[i] = std::chrono::steady_clock::now();
          continue;
        }

        pending[i] = false;

        if (value == last_published[i]) {
          continue;
        }
        last_published[i] = value;

        hexapod_custom_msgs::msg::SwitchEvent msg;
        msg.leg_index = static_cast<uint8_t>(i);
        msg.pressed   = (value == 0); // low level == pressed
        publisher_->publish(msg);
      }
    }
  } catch (const std::exception & e) {
    RCLCPP_FATAL(this->get_logger(), "GPIOD error in switch_node: %s", e.what());
    running_ = false;
    rclcpp::shutdown();
  }
}