#ifndef IMU_NODE_HPP_
#define IMU_NODE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <string>
#include <vector>
#include <array>
#include <cstdint>

// SHTP (Sensor Hub Transport Protocol)
constexpr uint8_t SHTP_START_FLAG  = 0x7E;
constexpr uint8_t SHTP_ESCAPE_CHAR = 0x7D;
constexpr uint8_t SHTP_ESCAPE_XOR  = 0x20;
constexpr uint8_t SHTP_PROTOCOL_ID = 0x01;

constexpr uint8_t SHTP_CHANNEL_COMMAND       = 0x00;
constexpr uint8_t SHTP_CHANNEL_EXECUTABLE    = 0x01;
constexpr uint8_t SHTP_CHANNEL_CONTROL       = 0x02;
constexpr uint8_t SHTP_CHANNEL_REPORTS       = 0x03;
constexpr uint8_t SHTP_CHANNEL_WAKE_REPORTS  = 0x04;

constexpr uint8_t SH2_ROTATION_VECTOR_REPORT_ID = 0x05;
constexpr uint8_t SHTP_EXEC_RESET               = 0x01;
constexpr uint8_t SH2_SET_FEATURE_CMD           = 0xFD;

// Frame / buffer sizes
constexpr size_t SHTP_HEADER_SIZE         = 4;    // length(2) + channel(1) + seq(1)
constexpr size_t SHTP_MAX_PACKET_SIZE     = 128;  // payload + header
constexpr size_t SHTP_MAX_RAW_SIZE        = 512;  // escaped data + margin
constexpr size_t SHTP_MAX_RX_BUFFER       = 4096; // safety limit
constexpr size_t SHTP_RX_CHUNK_SIZE       = 1024; // single read() size
constexpr size_t SHTP_RX_RESERVE          = 2048; // initial reserve for rx_buffer_
constexpr size_t SHTP_TX_OVERHEAD_BYTES   = 3;    // start flag + protocol ID + end flag
constexpr size_t SHTP_FRAME_MIN_SIZE      = 3;    // 0x7E + protocol ID + 0x7E
constexpr size_t SHTP_FEATURE_CARGO_SIZE  = 17;   // Set Feature command payload
constexpr int    SHTP_MAX_PACKETS_PER_TICK = 64;  // flood guard per timer tick

// Timing constants
constexpr int IMU_PUBLISH_INTERVAL_MS = 10;      // 100 Hz timer
constexpr int SHTP_RESET_DELAY_MS     = 300;
constexpr int SHTP_BYTE_DELAY_US      = 100;     // min gap between bytes on UART
constexpr int SHTP_REPORT_INTERVAL_US = 10000;   // 100 Hz report period

// Quaternion reported as int16 fixed point with scale 2^14
constexpr double SH2_QUATERNION_SCALE    = 16384.0;
constexpr int    kMaxConsecutiveFailures = 100;   // ~1 s without data → reinit


// Reads quaternion data from a BNO085 IMU over UART using the SHTP
class ImuNode : public rclcpp::Node {
public:
    static constexpr const char* DEFAULT_PORT_NAME = "/dev/ttyAMA0";

    explicit ImuNode(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~ImuNode() override;

private:
    void timer_callback();
    bool init_serial(const std::string& port_name);
    bool init_sensor();

    // SHTP framing / deframing
    bool send_shtp_packet(uint8_t channel, const uint8_t* payload, size_t payload_len);
    bool read_shtp_packet(uint8_t& out_channel, uint8_t* out_payload, size_t& out_len);
    bool process_rx_buffer(uint8_t& out_channel, uint8_t* out_payload, size_t& out_len);

    static size_t escape_bytes(const uint8_t* input, size_t input_len, uint8_t* output, size_t output_max);
    static size_t unescape_bytes(const uint8_t* input, size_t input_len, uint8_t* output, size_t output_max);
    static size_t sh2_report_length(uint8_t report_id);

    bool parse_rotation_vector(const uint8_t* cargo, size_t len, sensor_msgs::msg::Imu& msg);

    std::string port_name_;
    int serial_fd_ = -1;
    int consecutive_failures_ = 0;

    // SHTP requires an independent sequence number per channel (0–5)
    std::array<uint8_t, 6> sequence_number_{};

    // Buffer for efficient UART reading
    std::vector<uint8_t> rx_buffer_;

    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

#endif // IMU_NODE_HPP_