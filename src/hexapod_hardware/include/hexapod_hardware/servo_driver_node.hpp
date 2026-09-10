#ifndef SERVO_DRIVER_NODE_HPP_
#define SERVO_DRIVER_NODE_HPP_

#include <cstdint>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <gpiod.hpp>

// PCA9685 I2C addresses and prescaler for 50 Hz PWM
constexpr int PCA_ADDR_R = 0x40;
constexpr int PCA_ADDR_L = 0x41;
constexpr int PRESCALE_CONST = 121;

// Defaults for Raspberry Pi 5
constexpr const char* DEFAULT_GPIO_CHIP_SERVO = "gpiochip4";
constexpr int DEFAULT_OE_PIN = 4; // Output enable for servo power
constexpr const char* DEFAULT_I2C_BUS = "/dev/i2c-1";

struct ServoHw {
    int addr;
    int channel;
};

// Mapping of each joint to PCA board and channel
inline constexpr ServoHw hw_config[6][3] = {
    {{0x40, 0}, {0x40, 1}, {0x40, 2}},     // Leg 0 - L1
    {{0x40, 5}, {0x40, 6}, {0x40, 7}},     // Leg 1 - L2
    {{0x40, 12}, {0x40, 13}, {0x40, 14}},  // Leg 2 - L3
    {{0x41, 3}, {0x41, 2}, {0x41, 1}},     // Leg 3 - P3 
    {{0x41, 10}, {0x41, 9}, {0x41, 8}},    // Leg 4 - P2 
    {{0x41, 15}, {0x41, 14}, {0x41, 13}}   // Leg 5 - P1 
};

// Calibration PWM values at 0, 45, 90, 135, 180 degrees for each joint
inline constexpr float servo_calibration[6][3][5] = {
    // Leg 0 - L1
    { {119.f, 208.f, 307.f, 403.f, 504.f}, {114.f, 212.f, 315.f, 420.f, 521.f}, {109.f, 205.f, 308.f, 406.f, 501.f} },
    // Leg 1 - L2
    { {115.f, 210.f, 311.f, 411.f, 508.f}, {104.f, 206.f, 313.f, 417.f, 514.f}, {112.f, 206.f, 310.f, 414.f, 510.f} },
    // Leg 2 - L3
    { {110.f, 205.f, 310.f, 415.f, 511.f}, {104.f, 206.f, 315.f, 418.f, 515.f}, { 98.f, 191.f, 293.f, 397.f, 493.f} },
    // Leg 3 - P3
    { {109.f, 201.f, 299.f, 395.f, 490.f}, { 96.f, 196.f, 298.f, 400.f, 496.f}, {104.f, 197.f, 293.f, 390.f, 490.f} },
    // Leg 4 - P2
    { {106.f, 200.f, 300.f, 398.f, 494.f}, {111.f, 211.f, 310.f, 412.f, 508.f}, {102.f, 195.f, 295.f, 394.f, 489.f} },
    // Leg 5 - P1
    { {111.f, 203.f, 300.f, 399.f, 494.f}, {100.f, 200.f, 301.f, 402.f, 500.f}, {118.f, 207.f, 307.f, 403.f, 502.f} }
};
inline constexpr float kCalAnglesDeg[5] = {0.f, 45.f, 90.f, 135.f, 180.f};

// Receives hardware angles and drives 18 servos via two PCA9685
class ServoDriverNode : public rclcpp::Node {
public:
    explicit ServoDriverNode(const rclcpp::NodeOptions &options = rclcpp::NodeOptions());
    virtual ~ServoDriverNode();

private:
    int i2c_fd_ = -1;
    std::unique_ptr<gpiod::chip> gpio_chip_;
    gpiod::line oe_line_;
    
    // Cache of last written PWM values
    int prev_pwm_[6][3];

    // Mapping of each joint to PCA board and channel
    struct ChannelMap {
        int     addr;   // PCA_ADDR_R or PCA_ADDR_L
        uint8_t ch;     // 0..15
    };
    ChannelMap ch_map_[18];

    rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr angles_sub_;

    // I2C & PCA9685 low-level methods
    bool i2c_write_reg8(int addr, uint8_t reg, uint8_t value);
    uint8_t i2c_read_reg8(int addr, uint8_t reg);
    bool pca9685_init(int addr);
    void pca9685_sleep(int addr);
    void pca9685_all_channels_full_off(int addr);
    void pca9685_set_pwm_bulk(int addr, const uint16_t off_values[16]);

    int angle_to_pwm(float angle, int leg, int joint);

    void anglesCallback(const std_msgs::msg::Float32MultiArray & msg);
};

#endif // SERVO_DRIVER_NODE_HPP_