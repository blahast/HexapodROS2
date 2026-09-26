#include "hexapod_hardware/imu_node.hpp"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <cmath>
#include <algorithm>

// SH-2 report lengths (total bytes including the 1-byte report ID).
// Layout: ID (1B) + sequence/delay (4B) + data + accuracy (1B).
size_t ImuNode::sh2_report_length(uint8_t id) {
    switch (id) {
        case 0xFA: return 5;   // Timestamp Rebase
        case 0xFB: return 5;   // Base Timestamp Reference
        case 0x01: return 10;  // Accelerometer
        case 0x02: return 10;  // Gyroscope
        case 0x03: return 10;  // Magnetic Field
        case 0x04: return 10;  // Linear Acceleration
        case 0x05: return 14;  // Rotation Vector
        case 0x06: return 10;  // Gravity
        case 0x07: return 14;  // Uncalibrated Gyroscope
        case 0x08: return 14;  // Uncalibrated Magnetic Field
        case 0x09: return 14;  // Geomagnetic Rotation Vector
        case 0x0A: return 5;   // Tap Detector
        case 0x0B: return 5;   // Step Counter
        case 0x0C: return 5;   // Step Detector
        case 0x0D: return 5;   // Significant Motion
        case 0x0E: return 5;   // Stability Classifier
        case 0x0F: return 10;  // Raw Accelerometer
        case 0x10: return 10;  // Raw Gyroscope
        case 0x11: return 10;  // Raw Magnetometer
        case 0x12: return 14;  // ARVR Stabilized Rotation Vector
        case 0x13: return 14;  // ARVR Stabilized Game Rotation Vector
        case 0x14: return 14;  // Gyro Integrated Rotation Vector
        case 0xFE: return 5;   // Flush Completed
        case 0xFC: return 16;  // Command Response
        default:   return 0;   // unknown -> stop parsing
    }
}

ImuNode::ImuNode(const rclcpp::NodeOptions& options) : Node("imu_node", options) {
    this->declare_parameter("port", DEFAULT_PORT_NAME);
    port_name_ = this->get_parameter("port").as_string();

    if (!init_serial(port_name_)) {
        RCLCPP_ERROR(this->get_logger(), "Failed to open serial port: %s", port_name_.c_str());
        return;
    }

    if (!init_sensor()) {
        RCLCPP_WARN(this->get_logger(), "BNO085 initialization failed. Will retry later.");
    }

    publisher_ = this->create_publisher<sensor_msgs::msg::Imu>( "imu/data", rclcpp::QoS(1).best_effort());

    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(IMU_PUBLISH_INTERVAL_MS),
        [this]() { timer_callback(); });
}

ImuNode::~ImuNode() {
    if (serial_fd_ != -1) ::close(serial_fd_);
}

// Serial port initialization
bool ImuNode::init_serial(const std::string& port_name) {
    serial_fd_ = ::open(port_name.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_fd_ == -1) return false;

    struct termios options;
    if (tcgetattr(serial_fd_, &options) != 0) {
        ::close(serial_fd_);
        serial_fd_ = -1;
        return false;
    }

    // Raw mode at 3 Mbps
    cfmakeraw(&options);
    cfsetispeed(&options, B3000000);
    cfsetospeed(&options, B3000000);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~CRTSCTS;           // No hardware flow control
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_cc[VMIN]  = 0;               // Non-blocking
    options.c_cc[VTIME] = 0;

    if (tcsetattr(serial_fd_, TCSANOW, &options) != 0) {
        ::close(serial_fd_);
        serial_fd_ = -1;
        return false;
    }

    tcflush(serial_fd_, TCIOFLUSH);
    rx_buffer_.clear();
    rx_buffer_.reserve(SHTP_RX_RESERVE);
    sequence_number_.fill(0);
    consecutive_failures_ = 0;

    RCLCPP_INFO(this->get_logger(),
                "Serial port %s opened (3 Mbps, non-blocking).",
                port_name.c_str());
    return true;
}

// Escape / unescape
size_t ImuNode::escape_bytes(const uint8_t* input, size_t input_len, uint8_t* output, size_t output_max) {
    size_t out_idx = 0;
    for (size_t i = 0; i < input_len; ++i) {
        if (input[i] == SHTP_ESCAPE_CHAR || input[i] == SHTP_START_FLAG) {
            if (out_idx + 2 > output_max) return 0;
            output[out_idx++] = SHTP_ESCAPE_CHAR;
            output[out_idx++] = input[i] ^ SHTP_ESCAPE_XOR;
        } else {
            if (out_idx + 1 > output_max) return 0;
            output[out_idx++] = input[i];
        }
    }
    return out_idx;
}

size_t ImuNode::unescape_bytes(const uint8_t* input, size_t input_len,
                               uint8_t* output, size_t output_max) {
    size_t out_idx = 0;
    for (size_t i = 0; i < input_len; ++i) {
        if (input[i] == SHTP_ESCAPE_CHAR) {
            if (i + 1 >= input_len) return 0;   // Incomplete escape sequence
            ++i;
            if (out_idx >= output_max) return 0;
            output[out_idx++] = input[i] ^ SHTP_ESCAPE_XOR;
        } else {
            if (out_idx >= output_max) return 0;
            output[out_idx++] = input[i];
        }
    }
    return out_idx;
}

// Send one SHTP packet
bool ImuNode::send_shtp_packet(uint8_t channel, const uint8_t* payload, size_t payload_len) {
    if (serial_fd_ == -1) return false;
    if (payload_len + SHTP_HEADER_SIZE > SHTP_MAX_PACKET_SIZE) return false;

    uint8_t raw[SHTP_MAX_PACKET_SIZE];
    uint16_t total_len = static_cast<uint16_t>(SHTP_HEADER_SIZE + payload_len);

    // Header: length (LE), channel, per-channel sequence number
    raw[0] = static_cast<uint8_t>(total_len & 0xFF);
    raw[1] = static_cast<uint8_t>((total_len >> 8) & 0xFF);
    raw[2] = channel;
    raw[3] = sequence_number_[channel < sequence_number_.size() ? channel : 0]++;

    if (payload_len > 0) {
        std::memcpy(raw + SHTP_HEADER_SIZE, payload, payload_len);
    }

    uint8_t escaped[SHTP_MAX_RAW_SIZE];
    size_t escaped_len = escape_bytes(raw, total_len, escaped, sizeof(escaped));
    if (escaped_len == 0) return false;

    // Assemble the TX frame in one buffer
    std::vector<uint8_t> tx;
    tx.reserve(escaped_len + SHTP_TX_OVERHEAD_BYTES);
    tx.push_back(SHTP_START_FLAG);
    tx.push_back(SHTP_PROTOCOL_ID);
    tx.insert(tx.end(), escaped, escaped + escaped_len);
    tx.push_back(SHTP_START_FLAG);

    // BNO085 requires a gap of ≥ 100 µs between bytes
    for (size_t i = 0; i < tx.size(); ++i) {
        ssize_t w = ::write(serial_fd_, &tx[i], 1);
        if (w != 1) return false;
        if (i + 1 < tx.size()) {
            ::usleep(SHTP_BYTE_DELAY_US);
        }
    }
    return true;
}

// RX buffer processing – extract one SHTP packet
bool ImuNode::process_rx_buffer(uint8_t& out_channel, uint8_t* out_payload, size_t& out_len) {
    while (!rx_buffer_.empty()) {
        // Find the start flag
        auto start_it = std::find(rx_buffer_.begin(), rx_buffer_.end(), SHTP_START_FLAG);
        if (start_it == rx_buffer_.end()) {
            rx_buffer_.clear();
            return false;
        }
        if (start_it != rx_buffer_.begin()) {
            rx_buffer_.erase(rx_buffer_.begin(), start_it);
        }
        if (rx_buffer_.size() < SHTP_FRAME_MIN_SIZE) return false;  // 0x7E + proto + 0x7E

        // Find the ending flag
        auto end_it = std::find(rx_buffer_.begin() + 1, rx_buffer_.end(),
                                SHTP_START_FLAG);
        if (end_it == rx_buffer_.end()) return false;   // Frame not complete yet

        std::vector<uint8_t> frame(rx_buffer_.begin() + 1, end_it);
        // Keep the ending flag – it may be the start of the next frame
        rx_buffer_.erase(rx_buffer_.begin(), end_it);

        if (frame.size() < 2 || frame[0] != SHTP_PROTOCOL_ID) continue;

        uint8_t unescaped[SHTP_MAX_PACKET_SIZE];
        size_t unescaped_len = unescape_bytes(frame.data() + 1, frame.size() - 1, unescaped, sizeof(unescaped));
        if (unescaped_len < SHTP_HEADER_SIZE) continue;

        uint16_t pkt_len = static_cast<uint16_t>(unescaped[0] | (unescaped[1] << 8));
        uint8_t  channel = unescaped[2];

        if (pkt_len < SHTP_HEADER_SIZE || pkt_len > unescaped_len) continue;

        size_t cargo_len = pkt_len - SHTP_HEADER_SIZE;
        out_channel = channel;
        if (out_payload && cargo_len > 0) {
            std::memcpy(out_payload, unescaped + SHTP_HEADER_SIZE, cargo_len);
        }
        out_len = cargo_len;
        return true;
    }
    return false;
}

// Read one SHTP packet (first from buffer, then from UART)
bool ImuNode::read_shtp_packet(uint8_t& out_channel, uint8_t* out_payload, size_t& out_len) {
    if (serial_fd_ == -1) return false;

    // Try to process what we already have
    if (process_rx_buffer(out_channel, out_payload, out_len)) {
        return true;
    }

    // Drain the UART
    uint8_t buf[SHTP_RX_CHUNK_SIZE];
    ssize_t n = ::read(serial_fd_, buf, sizeof(buf));
    if (n > 0) {
        if (rx_buffer_.size() + static_cast<size_t>(n) > SHTP_MAX_RX_BUFFER) {
            RCLCPP_WARN(this->get_logger(),
                        "RX buffer overflow (%zu B), clearing.",
                        rx_buffer_.size());
            rx_buffer_.clear();
        }
        rx_buffer_.insert(rx_buffer_.end(), buf, buf + n);
    }

    // Retry parsing
    return process_rx_buffer(out_channel, out_payload, out_len);
}

// Sensor initialization
bool ImuNode::init_sensor() {
    if (serial_fd_ == -1) return false;

    // Soft reset
    const uint8_t reset_cargo[1] = { SHTP_EXEC_RESET };
    send_shtp_packet(SHTP_CHANNEL_EXECUTABLE, reset_cargo, sizeof(reset_cargo));
    ::usleep(SHTP_RESET_DELAY_MS * 1000);
    rx_buffer_.clear();
    tcflush(serial_fd_, TCIFLUSH);

    // Set Feature: Rotation Vector, period SHTP_REPORT_INTERVAL_US
    uint8_t feature_cargo[SHTP_FEATURE_CARGO_SIZE] = {0};
    feature_cargo[0] = SH2_SET_FEATURE_CMD;
    feature_cargo[1] = SH2_ROTATION_VECTOR_REPORT_ID;
    feature_cargo[2] = 0x00;   // Feature flags
    feature_cargo[3] = 0x00;   // Change sensitivity LSB
    feature_cargo[4] = 0x00;   // Change sensitivity MSB
    feature_cargo[5] = static_cast<uint8_t>((SHTP_REPORT_INTERVAL_US      ) & 0xFF);
    feature_cargo[6] = static_cast<uint8_t>((SHTP_REPORT_INTERVAL_US >>  8) & 0xFF);
    feature_cargo[7] = static_cast<uint8_t>((SHTP_REPORT_INTERVAL_US >> 16) & 0xFF);
    feature_cargo[8] = static_cast<uint8_t>((SHTP_REPORT_INTERVAL_US >> 24) & 0xFF);

    if (!send_shtp_packet(SHTP_CHANNEL_CONTROL, feature_cargo, sizeof(feature_cargo))) {
        return false;
    }

    RCLCPP_INFO(this->get_logger(),
                "BNO085 initialized (Rotation Vector @ 100 Hz).");
    return true;
}

// Parse Rotation Vector report(s) inside a single SH-2 cargo
bool ImuNode::parse_rotation_vector(const uint8_t* cargo, size_t len,
                                    sensor_msgs::msg::Imu& msg) {
    size_t i = 0;
    bool parsed = false;

    while (i < len) {
        uint8_t report_id  = cargo[i];
        size_t  report_len = sh2_report_length(report_id);

        if (report_len == 0) {
            // Unknown report – cannot skip it safely, stop parsing
            break;
        }

        if (report_id == SH2_ROTATION_VECTOR_REPORT_ID) {
            if (i + report_len > len) return parsed;

            // Quaternion components are int16 little-endian (Q14 fixed point)
            int16_t q_i    = static_cast<int16_t>((cargo[i + 5] << 8) | cargo[i + 4]);
            int16_t q_j    = static_cast<int16_t>((cargo[i + 7] << 8) | cargo[i + 6]);
            int16_t q_k    = static_cast<int16_t>((cargo[i + 9] << 8) | cargo[i + 8]);
            int16_t q_real = static_cast<int16_t>((cargo[i + 11] << 8) | cargo[i + 10]);

            msg.orientation.w = q_real / SH2_QUATERNION_SCALE;
            msg.orientation.x = q_i    / SH2_QUATERNION_SCALE;
            msg.orientation.y = q_j    / SH2_QUATERNION_SCALE;
            msg.orientation.z = q_k    / SH2_QUATERNION_SCALE;

            parsed = true;
        }

        i += report_len;
    }

    return parsed;
}

// Periodic callback
void ImuNode::timer_callback() {
    if (serial_fd_ == -1) {
        // Attempt to reopen the port
        if (init_serial(port_name_) && init_sensor()) {
            RCLCPP_INFO(this->get_logger(), "Port reinitialization successful.");
        }
        return;
    }

    uint8_t channel   = 0;
    uint8_t cargo[SHTP_MAX_PACKET_SIZE];
    size_t  cargo_len = 0;
    bool    activity  = false;
    int     packets_this_tick = 0;

    // Process all currently available packets
    while (read_shtp_packet(channel, cargo, cargo_len)) {
        activity = true;

        if ((channel == SHTP_CHANNEL_REPORTS ||
             channel == SHTP_CHANNEL_WAKE_REPORTS) && cargo_len > 0) {

            sensor_msgs::msg::Imu msg;
            msg.header.stamp    = this->now();
            msg.header.frame_id = "imu_link";

            if (parse_rotation_vector(cargo, cargo_len, msg)) {
                // Angular velocity and linear acceleration not provided by this path
                msg.angular_velocity_covariance[0]    = -1.0;
                msg.linear_acceleration_covariance[0] = -1.0;
                publisher_->publish(msg);
            }
        }

        // Guard against endless looping when the buffer is flooded
        if (++packets_this_tick > SHTP_MAX_PACKETS_PER_TICK) break;
    }

    // Timeout detection
    if (activity) {
        consecutive_failures_ = 0;
    } else if (++consecutive_failures_ >= kMaxConsecutiveFailures) {
        RCLCPP_WARN(this->get_logger(), "IMU silent for too long, reinitializing...");
        ::close(serial_fd_);
        serial_fd_ = -1;
        if (init_serial(port_name_) && init_sensor()) {
            RCLCPP_INFO(this->get_logger(), "Reinitialization successful.");
        }
        consecutive_failures_ = 0;
    }
}