#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <cstring>
#include <cmath>

#include "hexapod_hardware/imu_node.hpp"

ImuNode::ImuNode(const rclcpp::NodeOptions & options) : Node("imu_node", options), serial_fd_(-1)
{
    this->declare_parameter("port", DEFAULT_PORT_NAME);
    port_name_ = this->get_parameter("port").as_string();

    if (!init_serial(port_name_)) {
        RCLCPP_ERROR(this->get_logger(), "Failed to open serial port: %s", port_name_.c_str());
        return;
    }

    // BNO055 must be in NDOF (9‑DOF fusion) mode to output quaternions
    if (!set_ndof_mode()) {
        RCLCPP_ERROR(this->get_logger(), "Failed to switch BNO055 to NDOF mode.");
    }

    publisher_ = this->create_publisher<sensor_msgs::msg::Imu>(
        "imu/data", rclcpp::QoS(1).best_effort());

    // Poll IMU at fixed rate (see IMU_PUBLISH_INTERVAL_MS)
    timer_ = this->create_wall_timer(
        std::chrono::milliseconds(IMU_PUBLISH_INTERVAL_MS),
        std::bind(&ImuNode::timer_callback, this));
}

ImuNode::~ImuNode() {
    if (serial_fd_ != -1) {
        close(serial_fd_);
    }
}

bool ImuNode::init_serial(const std::string& port_name) {
    serial_fd_ = open(port_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd_ == -1) return false;

    struct termios options;
    if (tcgetattr(serial_fd_, &options) != 0) {
        close(serial_fd_);
        serial_fd_ = -1;
        return false;
    }

    // BNO055 default UART speed
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);

    // 8N1, no flow control, raw mode
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;
    options.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    // VTIME=1 → 100 ms read timeout (used as a fallback below)
    options.c_cc[VMIN]  = 0;
    options.c_cc[VTIME] = 1;

    if (tcsetattr(serial_fd_, TCSANOW, &options) != 0) {
        close(serial_fd_);
        serial_fd_ = -1;
        return false;
    }
    fcntl(serial_fd_, F_SETFL, 0);
    return true;
}

bool ImuNode::set_ndof_mode() {
    // Pomocná lambda pro odeslání příkazu a bezpečné iterativní vyčtení ACK
    auto send_and_check = [&](uint8_t mode, const char* mode_name, int delay_us) {
        uint8_t cmd[BNO055_WRITE_CMD_SIZE] = {
            BNO055_START_BYTE, BNO055_WRITE_OP, BNO055_OPR_MODE_ADDR, 0x01, mode
        };
        
        // Vyčištění starých dat v lince před odesláním nového příkazu
        tcflush(serial_fd_, TCIOFLUSH); 
        
        if (write(serial_fd_, cmd, BNO055_WRITE_CMD_SIZE) != BNO055_WRITE_CMD_SIZE) {
            RCLCPP_ERROR(this->get_logger(), "Failed to write command for %s mode", mode_name);
            return false;
        }
        
        usleep(delay_us); 
        
        uint8_t resp[2] = {0};
        int total_read = 0;
        int attempts = 10;
        
        // Iterativní čtení: zachytí fragmentované UART pakety
        while (total_read < 2 && attempts > 0) {
            ssize_t bytes = read(serial_fd_, resp + total_read, 2 - total_read);
            if (bytes > 0) {
                total_read += bytes;
            } else {
                usleep(2000); // 2 ms pauza, pokud druhy bajt ještě cestuje po kabelu
            }
            attempts--;
        }
        
        if (total_read == 2) {
            if (resp[0] == 0xEE && resp[1] == 0x01) {
                return true; // 0x01 = Úspěšný zápis
            }
            // Pokud senzor odmítne příkaz, vypíšeme přesný chybový kód
            RCLCPP_ERROR(this->get_logger(), "BNO055 refused %s mode. Bus error: 0x%02X 0x%02X", 
                         mode_name, resp[0], resp[1]);
            return false;
        }
        
        RCLCPP_ERROR(this->get_logger(), "BNO055 timeout setting %s mode. Read only %d bytes.", 
                     mode_name, total_read);
        return false;
    };

    // 1. Krok: Přechod do CONFIG módu (povinné před změnou na NDOF)
    if (!send_and_check(OPERATION_MODE_CONFIG, "CONFIG", BNO055_MODE_SWITCH_DELAY_US)) {
        return false;
    }

    // 2. Krok: Přechod do NDOF (Sensor Fusion)
    if (!send_and_check(OPERATION_MODE_NDOF, "NDOF", BNO055_MODE_SWITCH_DELAY_NDOF_US)) {
        return false;
    }
    
    return true;
}

void ImuNode::timer_callback() {
    if (serial_fd_ == -1) return;

    uint8_t read_cmd[BNO055_READ_CMD_SIZE] = {
        BNO055_START_BYTE, BNO055_READ_OP, BNO055_QUATERNION_DATA_W_LSB_ADDR, BNO055_QUATERNION_DATA_LENGTH
    };
    tcflush(serial_fd_, TCIFLUSH);
    ssize_t written = write(serial_fd_, read_cmd, BNO055_READ_CMD_SIZE);
    
    if (written != static_cast<ssize_t>(BNO055_READ_CMD_SIZE)) return;

    uint8_t buffer[BNO055_RESPONSE_SIZE];
    int total_bytes_read = 0;
    int attempts_left = BNO055_MAX_READ_ATTEMPTS;

    while (total_bytes_read < 2 && attempts_left > 0) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(serial_fd_, &read_fds);
        struct timeval timeout = {0, BNO055_READ_TIMEOUT_US};

        if (select(serial_fd_ + 1, &read_fds, NULL, NULL, &timeout) > 0) {
            ssize_t bytes = read(serial_fd_, buffer + total_bytes_read, 2 - total_bytes_read);
            if (bytes > 0) total_bytes_read += static_cast<int>(bytes);
        } else {
            attempts_left--;
        }
    }

    if (total_bytes_read == 2) {
        if (buffer[0] == 0xEE) {
            RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), LOG_THROTTLE_MS, "BNO055 Bus Error: 0x%02X", buffer[1]);
            consecutive_failures_++;
            return; 
        }

        if (buffer[0] == BNO055_RESPONSE_BYTE && buffer[1] == BNO055_QUATERNION_DATA_LENGTH) {
            while (total_bytes_read < static_cast<int>(BNO055_RESPONSE_SIZE) && attempts_left > 0) {
                fd_set read_fds;
                FD_ZERO(&read_fds);
                FD_SET(serial_fd_, &read_fds);
                struct timeval timeout = {0, BNO055_READ_TIMEOUT_US};

                if (select(serial_fd_ + 1, &read_fds, NULL, NULL, &timeout) > 0) {
                    ssize_t bytes = read(serial_fd_, buffer + total_bytes_read, BNO055_RESPONSE_SIZE - total_bytes_read);
                    if (bytes > 0) total_bytes_read += static_cast<int>(bytes);
                } else {
                    attempts_left--;
                }
            }
        }
    }

    bool ok = false;
    // Validate response header (start byte + expected length)
    if (total_bytes_read == static_cast<int>(BNO055_RESPONSE_SIZE) &&
        buffer[0] == BNO055_RESPONSE_BYTE &&
        buffer[1] == BNO055_QUATERNION_DATA_LENGTH) {

        auto msg = sensor_msgs::msg::Imu();
        msg.header.stamp = this->now();
        msg.header.frame_id = "imu_link";

        // Quaternion components are little‑endian int16 (W, X, Y, Z)
        int16_t w = (buffer[3] << 8) | buffer[2];
        int16_t x = (buffer[5] << 8) | buffer[4];
        int16_t y = (buffer[7] << 8) | buffer[6];
        int16_t z = (buffer[9] << 8) | buffer[8];

        // BNO055 outputs quaternions scaled by 2^14
        msg.orientation.w = w / BNO055_QUATERNION_SCALE;
        msg.orientation.x = x / BNO055_QUATERNION_SCALE;
        msg.orientation.y = y / BNO055_QUATERNION_SCALE;
        msg.orientation.z = z / BNO055_QUATERNION_SCALE;

        // Mark angular velocity and linear acceleration as unavailable
        msg.angular_velocity_covariance[0]    = -1.0;
        msg.linear_acceleration_covariance[0] = -1.0;

        publisher_->publish(msg);
        ok = true;
    } else {
        // Discard stale bytes so the next read starts cleanly
        tcflush(serial_fd_, TCIFLUSH);
    }

    if (ok) {
        consecutive_failures_ = 0;
    } else if (++consecutive_failures_ >= kMaxConsecutiveFailures) {
        // Recover from a stuck/absent sensor by re‑opening the port
        RCLCPP_WARN(this->get_logger(), "IMU not responding, re-initializing port.");
        close(serial_fd_);
        serial_fd_ = -1;

        if (init_serial(port_name_) && set_ndof_mode()) {
            RCLCPP_INFO(this->get_logger(), "IMU re-initialization successful.");
        } else {
            RCLCPP_ERROR(this->get_logger(), "IMU re-initialization failed.");
        }
        consecutive_failures_ = 0;
    }
}