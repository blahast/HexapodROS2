#ifndef IMU_NODE_HPP_
#define IMU_NODE_HPP_

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <string>

// BNO055 protocol constants
constexpr uint8_t BNO055_START_BYTE     = 0xAA;  // host -> sensor
constexpr uint8_t BNO055_RESPONSE_BYTE  = 0xBB;  // sensor -> host
constexpr uint8_t BNO055_WRITE_OP       = 0x00;  // write operation
constexpr uint8_t BNO055_READ_OP        = 0x01;  // read operation

// Register addresses
constexpr uint8_t BNO055_OPR_MODE_ADDR                = 0x3D;
constexpr uint8_t BNO055_QUATERNION_DATA_W_LSB_ADDR   = 0x20;

// Operation modes
constexpr uint8_t OPERATION_MODE_CONFIG = 0x00;
constexpr uint8_t OPERATION_MODE_NDOF   = 0x0C;

// Frame sizes
constexpr size_t BNO055_WRITE_CMD_SIZE       = 5;   // start + op + addr + len + data
constexpr size_t BNO055_READ_CMD_SIZE        = 4;   // start + op + addr + len
constexpr size_t BNO055_RESPONSE_SIZE        = 10;  // header + 8 bytes quaternion

// Payload length
constexpr uint8_t BNO055_QUATERNION_DATA_LENGTH = 0x08;

// Timing and behaviour constants
constexpr int IMU_PUBLISH_INTERVAL_MS        = 10;      // timer period
constexpr int BNO055_MODE_SWITCH_DELAY_US    = 25000;   // wait after entering CONFIG
constexpr int BNO055_MODE_SWITCH_DELAY_NDOF_US = 30000; // wait after entering NDOF
constexpr int BNO055_READ_TIMEOUT_US         = 2000;    // per‑attempt select() timeout
constexpr int BNO055_MAX_READ_ATTEMPTS       = 5;       // partial‑read retry budget

// Quaternion output scale (BNO055 reports int16 = q * 2^14)
constexpr double BNO055_QUATERNION_SCALE = 16384.0;

// Error handling
constexpr int kMaxConsecutiveFailures = 20;   // triggers port re‑init
constexpr int LOG_THROTTLE_MS         = 2000; // throttle for warning logs


// Reads quaternion data from a BNO055 IMU over a serial port

class ImuNode : public rclcpp::Node {
public:
    static constexpr const char* DEFAULT_PORT_NAME = "/dev/ttyAMA0";
    explicit ImuNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());
    ~ImuNode() override;

private:
    void timer_callback();
    bool init_serial(const std::string& port_name);
    bool set_ndof_mode();

    std::string port_name_;
    int consecutive_failures_ = 0;

    int serial_fd_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
};

#endif // IMU_NODE_HPP_