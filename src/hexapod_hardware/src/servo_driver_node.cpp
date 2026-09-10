#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <stdexcept>

#include "hexapod_hardware/servo_driver_node.hpp"

ServoDriverNode::ServoDriverNode(const rclcpp::NodeOptions &options) 
    : Node("servo_driver_node", options) 
{
    // Initialise PWM cache to -1
    for(int l=0; l<6; ++l)
        for(int j=0; j<3; ++j)
            prev_pwm_[l][j] = -1;

    this->declare_parameter("i2c_bus", DEFAULT_I2C_BUS);
    this->declare_parameter("gpio_chip", DEFAULT_GPIO_CHIP_SERVO);
    this->declare_parameter("oe_pin", DEFAULT_OE_PIN);

    std::string i2c_bus = this->get_parameter("i2c_bus").as_string();
    std::string gpio_chip = this->get_parameter("gpio_chip").as_string();
    int oe_pin = this->get_parameter("oe_pin").as_int();

    // Open I2C bus
    i2c_fd_ = open(i2c_bus.c_str(), O_RDWR);
    if (i2c_fd_ < 0) {
        RCLCPP_ERROR(this->get_logger(), "Failed to open I2C bus: %s", i2c_bus.c_str());
        throw std::runtime_error("I2C open failed");
    }

    // Open GPIO chip and get OE line
    try {
        gpio_chip_ = std::make_unique<gpiod::chip>(gpio_chip);
        oe_line_ = gpio_chip_->get_line(oe_pin);

        // Request OE line as output, initially high (default_val = 1)
        oe_line_.request({"hexapod_servo", gpiod::line_request::DIRECTION_OUTPUT, 0}, 1);
    } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize GPIO for servo: %s", e.what());
        throw std::runtime_error("GPIO initialization failed");
    }

    // Initialise both PCA9685 boards
    if (!pca9685_init(PCA_ADDR_R) || !pca9685_init(PCA_ADDR_L)) {
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize PCA9685 boards");
        throw std::runtime_error("PCA9685 init failed");
    }

    // Enable servos
    oe_line_.set_value(0);

    // Initialize channel mapping
    for (int l = 0; l < 6; ++l) {
        for (int j = 0; j < 3; ++j) {
            const int idx       = l * 3 + j;
            ch_map_[idx].addr   = hw_config[l][j].addr;
            ch_map_[idx].ch     = static_cast<uint8_t>(hw_config[l][j].channel);
        }
    }

    // Subscribe to angles topic
    angles_sub_ = this->create_subscription<std_msgs::msg::Float32MultiArray>(
        "locomotion/hardware_angles",
        rclcpp::QoS(10).best_effort(),
        std::bind(&ServoDriverNode::anglesCallback, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "Servo driver initialized. PCA9685 boards ready.");
}

ServoDriverNode::~ServoDriverNode() {
    RCLCPP_INFO(this->get_logger(), "Shutting down servos...");
    // Disable servos
    if (oe_line_) {
        oe_line_.set_value(1);
        usleep(100000);
        oe_line_.release();
    }
    if (i2c_fd_ >= 0) {
        // Turn off all channels and put PCA9685 to sleep
        pca9685_all_channels_full_off(PCA_ADDR_R);
        pca9685_all_channels_full_off(PCA_ADDR_L);
        pca9685_sleep(PCA_ADDR_R);
        pca9685_sleep(PCA_ADDR_L);
        close(i2c_fd_);
    }
}

// Write a byte to a register
bool ServoDriverNode::i2c_write_reg8(int addr, uint8_t reg, uint8_t value) {
    if (ioctl(i2c_fd_, I2C_SLAVE, addr) < 0) return false;
    uint8_t buffer[2] = {reg, value};
    return write(i2c_fd_, buffer, 2) == 2;
}

// Read a byte from a register
uint8_t ServoDriverNode::i2c_read_reg8(int addr, uint8_t reg) {
    if (ioctl(i2c_fd_, I2C_SLAVE, addr) < 0) return 0;
    if (write(i2c_fd_, &reg, 1) != 1) return 0;
    uint8_t value = 0;
    if (read(i2c_fd_, &value, 1) != 1) return 0;
    return value;
}

// Initialize PCA9685
bool ServoDriverNode::pca9685_init(int addr) {
    // Enable auto-increment, sleep
    if (!i2c_write_reg8(addr, 0x01, 0x04 | 0x08)) return false;
    uint8_t mode1 = i2c_read_reg8(addr, 0x00);
    // Set sleep bit to allow prescaler write
    if (!i2c_write_reg8(addr, 0x00, (mode1 & ~0x80) | 0x10)) return false;
    usleep(500);
    // Set prescaleer for 50 Hz
    if (!i2c_write_reg8(addr, 0xFE, PRESCALE_CONST)) return false;
    usleep(500);
    // Clear sleep, enable auto-increment
    mode1 = (mode1 & ~0x10) | 0x20;
    if (!i2c_write_reg8(addr, 0x00, mode1)) return false;
    usleep(500);
    // Restart
    if (!i2c_write_reg8(addr, 0x00, mode1 | 0x80)) return false;
    usleep(500);
    return true;
}

// Put PCA9685 into sleep mode
void ServoDriverNode::pca9685_sleep(int addr) {
    uint8_t mode1 = i2c_read_reg8(addr, 0x00);
    i2c_write_reg8(addr, 0x00, mode1 | 0x10);
}

// Turn off all 16 channels
void ServoDriverNode::pca9685_all_channels_full_off(int addr) {
    if (ioctl(i2c_fd_, I2C_SLAVE, addr) < 0) return;
    uint8_t buffer[2] = {0xFD, 0x10}; 
    if (write(i2c_fd_, buffer, 2) != 2) {
        RCLCPP_WARN(this->get_logger(), "I2C bulk off write failed for addr 0x%02X", addr);
    }
}

// Write PWM values for all 16 channels
void ServoDriverNode::pca9685_set_pwm_bulk(int addr, const uint16_t off_values[16]) {
    if (ioctl(i2c_fd_, I2C_SLAVE, addr) < 0) return;
    uint8_t buffer[65];
    buffer[0] = 0x06; // start at LED0_ON_L
    for (int i = 0; i < 16; i++) {
        buffer[1 + i*4] = 0x00;                        // LED_ON_L = 0
        buffer[2 + i*4] = 0x00;                        // LED_ON_H = 0
        buffer[3 + i*4] = off_values[i] & 0xFF;        // LED_OFF_L
        buffer[4 + i*4] = (off_values[i] >> 8) & 0x0F; // LED_OFF_H
    }
    if (write(i2c_fd_, buffer, 65) != 65) {
        RCLCPP_WARN(this->get_logger(), "I2C bulk write failed for addr 0x%02X", addr);
    }
}

// Convert angle (deg) to PWM
int ServoDriverNode::angle_to_pwm(float angle, int leg, int joint) {
    int i = static_cast<int>(angle / 45.0f);
    if (i < 0) i = 0;
    if (i > 3) i = 3;
    
    float t = (angle - kCalAnglesDeg[i]) / 45.0f;
    const float *p = servo_calibration[leg][joint];
    
    return std::lround(p[i] + (p[i+1] - p[i]) * t);
}

// Receives 18 angles, converts to PWM, writes only if changed
void ServoDriverNode::anglesCallback(const std_msgs::msg::Float32MultiArray & msg)
{
    if (msg.data.size() < 18) return;

    uint16_t pwm_r[16] = {0};
    uint16_t pwm_l[16] = {0};
    bool update_r = false;
    bool update_l = false;

    const float * data = msg.data.data();

    for (int idx = 0; idx < 18; ++idx) {
        const int l = idx / 3;
        const int j = idx % 3;

        const int pwm = angle_to_pwm(data[idx], l, j);

        uint16_t * buf = (ch_map_[idx].addr == PCA_ADDR_R) ? pwm_r : pwm_l;
        buf[ch_map_[idx].ch] = static_cast<uint16_t>(pwm);

        if (pwm != prev_pwm_[l][j]) {
            prev_pwm_[l][j] = pwm;
            if (ch_map_[idx].addr == PCA_ADDR_R) update_r = true;
            else                                  update_l = true;
        }
    }

    if (update_r) pca9685_set_pwm_bulk(PCA_ADDR_R, pwm_r);
    if (update_l) pca9685_set_pwm_bulk(PCA_ADDR_L, pwm_l);
}