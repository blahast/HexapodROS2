#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <cmath>
#include <cstring>      // for memmove, strerror
#include <chrono>
#include <cerrno>

#include "hexapod_teleop/bluetooth_teleop_node.hpp"

BluetoothTeleopNode::BluetoothTeleopNode(const rclcpp::NodeOptions & options) 
    : Node("bluetooth_teleop", options) {

    // ROS parameters for port and joystick calibration
    this->declare_parameter<std::string>("port_name", DEFAULT_SERIAL_PORT);
    this->declare_parameter<int>("baud_rate", DEFAULT_BAUD_RATE);
    this->declare_parameter<float>("joy_center", DEFAULT_JOYSTICK_CENTER);
    this->declare_parameter<float>("joy_deadzone", DEFAULT_JOYSTICK_DEADZONE);

    port_name_ = this->get_parameter("port_name").as_string();
    baud_rate_ = this->get_parameter("baud_rate").as_int();
    joystick_center_value_ = this->get_parameter("joy_center").as_double();
    joystick_deadzone_ = this->get_parameter("joy_deadzone").as_double();

    // For mapping potentiometer readings [0, 1023] to [0, 1]
    potentiometer_denominator_ = (joystick_center_value_ * 2.0f) - 1.0f;

    // Reliable QoS for commands and state feedback, best effort for high frequency motion data
    auto best_effort_qos = rclcpp::QoS(1).best_effort();
    auto reliable_qos = rclcpp::QoS(10).reliable();

    // Publishers for teleop output
    velocity_publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", best_effort_qos);
    pose_publisher_ = this->create_publisher<geometry_msgs::msg::Pose>("body_pose", best_effort_qos);
    walking_params_publisher_ = this->create_publisher<std_msgs::msg::Float32MultiArray>("walking_params", best_effort_qos);
    event_publisher_ = this->create_publisher<hexapod_custom_msgs::msg::TeleopEvent>("teleop_events", reliable_qos);

    RCLCPP_INFO(this->get_logger(), "Bluetooth teleop node initialized. Port: %s at %d baud.", port_name_.c_str(), baud_rate_);
    
    is_running_ = true;
    serial_file_descriptor_ = -1;
    serial_thread_ = std::thread(&BluetoothTeleopNode::serialReadLoop, this);
}

// Shut down the thread
BluetoothTeleopNode::~BluetoothTeleopNode() {
    is_running_ = false;
    
    // Close the port to interrupt blocking poll() in the thread
    int fd = serial_file_descriptor_.exchange(-1);
    if (fd >= 0) {
        close(fd);
    }
    
    if (serial_thread_.joinable()) {
        serial_thread_.join();
    }
}

// Convert integer baud rate to termios macro
speed_t BluetoothTeleopNode::getBaudRateMacro(int baud) {
    switch(baud) {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        default: 
            RCLCPP_WARN(this->get_logger(), "Unsupported baud rate %d, defaulting to 115200", baud);
            return B115200;
    }
}

// Set up the serial port in raw mode
void BluetoothTeleopNode::configureSerialRawMode(struct termios* tty_config) {
    tty_config->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty_config->c_oflag &= ~OPOST;
    tty_config->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty_config->c_cflag &= ~(CSIZE | PARENB);
    tty_config->c_cflag |= CS8;
}

// Open and configure the serial, returns file descriptor or -1
int BluetoothTeleopNode::openSerialPort(const char* device_path, int baud_rate) {
    int fd = open(device_path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) return -1;

    struct termios tty_config;
    if (tcgetattr(fd, &tty_config) != 0) {
        close(fd);
        return -1;
    }
    
    configureSerialRawMode(&tty_config);
    speed_t baud_macro = getBaudRateMacro(baud_rate);
    cfsetispeed(&tty_config, baud_macro);
    cfsetospeed(&tty_config, baud_macro);
    tty_config.c_cflag |= (CLOCAL | CREAD);
    tty_config.c_cflag &= ~CSTOPB;
    tty_config.c_cc[VMIN] = 0;
    tty_config.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty_config) != 0) {
        close(fd);
        return -1;
    }
    tcflush(fd, TCIOFLUSH);
    return fd;
}

// Sum of all bytes in the frame (excluding the checksum)
uint8_t BluetoothTeleopNode::calculateChecksum(const uint8_t* buffer, size_t length) {
    uint32_t sum = 0;
    for (size_t i = 0; i < length; i++) sum += buffer[i];
    return static_cast<uint8_t>(sum & 0xFF);
}

// Map [0, 1023] joystick value to [-1, 1], apply deadzone, optional inversion
float BluetoothTeleopNode::normalizeJoystick(uint16_t raw_value, bool invert) {
    float difference = static_cast<float>(raw_value) - joystick_center_value_;
    if (std::abs(difference) < joystick_deadzone_) return 0.0f;
    
    float normalized = difference / joystick_center_value_; 
    if (normalized > 1.0f) normalized = 1.0f;
    if (normalized < -1.0f) normalized = -1.0f;
    
    return invert ? -normalized : normalized;
}

// Map potentiometer value [0, 1023] to [0, 1]
float BluetoothTeleopNode::normalizePotentiometer(uint16_t raw_value) {
    float normalized = static_cast<float>(raw_value) / potentiometer_denominator_;
    if (normalized > 1.0f) normalized = 1.0f;
    if (normalized < 0.0f) normalized = 0.0f;
    return normalized;
}

// Decode payload, detect button presses, publish messages
void BluetoothTeleopNode::processTelemetryPacket(const uint8_t* payload) {
    uint16_t analog_inputs[8];
    
    // Payload is little‑endian
    for (int i = 0; i < 8; i++) {
        analog_inputs[i] = static_cast<uint16_t>(payload[i*2] | (payload[i*2 + 1] << 8));
    }
    uint8_t current_buttons = payload[16];

    // Channel mapping
    uint16_t joy1_y = analog_inputs[0]; 
    uint16_t joy1_x = analog_inputs[1];
    uint16_t joy2_y = analog_inputs[2]; 
    uint16_t joy2_x = analog_inputs[3];
    uint16_t pot_3 = analog_inputs[4]; 
    uint16_t pot_4 = analog_inputs[5];
    uint16_t pot_1 = analog_inputs[6]; 
    uint16_t pot_2 = analog_inputs[7];

    // Detect rising edges
    bool is_joy1_pressed = (current_buttons & (1<<0)) && !(previous_button_state_ & (1<<0));
    bool is_joy2_pressed = (current_buttons & (1<<1)) && !(previous_button_state_ & (1<<1));
    bool is_btn1_pressed = (current_buttons & (1<<2)) && !(previous_button_state_ & (1<<2));
    bool is_btn2_pressed = (current_buttons & (1<<3)) && !(previous_button_state_ & (1<<3));
    bool is_btn3_pressed = (current_buttons & (1<<4)) && !(previous_button_state_ & (1<<4));

    previous_button_state_ = current_buttons;

    // Toggle operational modes
    if (is_joy1_pressed) is_crab_walk_active_ = !is_crab_walk_active_;
    if (is_joy2_pressed) is_rpy_offset_active_ = !is_rpy_offset_active_;

    // Send teleop events
    if (is_btn1_pressed) publishTeleopEvent(hexapod_custom_msgs::msg::TeleopEvent::CHANGE_GAIT);
    if (is_btn2_pressed) publishTeleopEvent(hexapod_custom_msgs::msg::TeleopEvent::TOGGLE_STAND);
    if (is_btn3_pressed) publishTeleopEvent(hexapod_custom_msgs::msg::TeleopEvent::PLAY_ANIMATION);

    // Velocity linear.x always from joy1_x, linear.y or angular.z depends on crab mode
    geometry_msgs::msg::Twist velocity_msg;
    float norm_vel_x = normalizeJoystick(joy1_x, true);
    float norm_vel_y = normalizeJoystick(joy1_y, true);

    velocity_msg.linear.x = norm_vel_x;
    if (is_crab_walk_active_) {
        velocity_msg.linear.y = norm_vel_y;      // Lateral movement
        velocity_msg.angular.z = 0.0;
    } else {
        velocity_msg.linear.y = 0.0;
        velocity_msg.angular.z = norm_vel_y;     // Rotation
    }
    velocity_publisher_->publish(velocity_msg);

    // Body pose: pot_3 = z, pot_4 = yaw
    geometry_msgs::msg::Pose pose_msg;
    float pos_x = 0, pos_y = 0, pos_z = 0;
    float roll = 0, pitch = 0, yaw = 0;

    pos_z = normalizeJoystick(pot_3);
    yaw = normalizeJoystick(pot_4);

    // joy2 controls either position (x, y) or orientation (roll, pitch) depends on mode
    if (is_rpy_offset_active_) {
        pitch = normalizeJoystick(joy2_x, true);
        roll = normalizeJoystick(joy2_y);
    } else {
        pos_x = normalizeJoystick(joy2_x, true);
        pos_y = normalizeJoystick(joy2_y, true);
    }

    pose_msg.position.x = pos_x;
    pose_msg.position.y = pos_y;
    pose_msg.position.z = pos_z;

    // Convert RPY to quaternion
    tf2::Quaternion quaternion;
    quaternion.setRPY(roll, pitch, yaw);
    pose_msg.orientation.x = quaternion.x();
    pose_msg.orientation.y = quaternion.y();
    pose_msg.orientation.z = quaternion.z();
    pose_msg.orientation.w = quaternion.w();

    pose_publisher_->publish(pose_msg);

    // Walking parameters: two potentiometers (gait speed, step height)
    std_msgs::msg::Float32MultiArray params_msg;
    params_msg.data.push_back(normalizePotentiometer(pot_1));
    params_msg.data.push_back(normalizePotentiometer(pot_2));
    walking_params_publisher_->publish(params_msg);
}

// Publish a teleop event
void BluetoothTeleopNode::publishTeleopEvent(uint8_t event_id) {
    hexapod_custom_msgs::msg::TeleopEvent msg;
    msg.event_id = event_id;
    event_publisher_->publish(msg);
}

// Main serial reading thread
void BluetoothTeleopNode::serialReadLoop() {
    bool is_connected = false;

    while (is_running_) {
        int fd = openSerialPort(port_name_.c_str(), baud_rate_);
        if (fd < 0) {
            // Retry every second if device not available
            // Sleep in 100 ms intervals to detect shutdown quickly
            for (int i = 0; i < 10 && is_running_; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            continue;
        }

        serial_file_descriptor_ = fd; 
        uint8_t rx_buffer[512];
        size_t buffer_length = 0;

        while (is_running_) {
            struct pollfd poll_fd;
            poll_fd.fd = serial_file_descriptor_;
            poll_fd.events = POLLIN;

            // Wait for data with a 2 second timeout
            int poll_result = poll(&poll_fd, 1, 2000);

            if (poll_result < 0) {
                if (errno == EINTR) continue;
                break;
            } else if (poll_result == 0) {
                // No data within timeout
                if (is_connected) {
                    RCLCPP_WARN(this->get_logger(), "Bluetooth connection lost (timeout). Stopping robot...");
                    publishTeleopEvent(hexapod_custom_msgs::msg::TeleopEvent::BT_DISCONNECTED);
                    is_connected = false;
                }
                continue; 
            }

            if (poll_fd.revents & POLLIN) {
                ssize_t bytes_read = read(serial_file_descriptor_, rx_buffer + buffer_length, sizeof(rx_buffer) - buffer_length);

                if (bytes_read > 0) {
                    if (!is_connected) {
                        RCLCPP_INFO(this->get_logger(), "Bluetooth connection established!");
                        publishTeleopEvent(hexapod_custom_msgs::msg::TeleopEvent::BT_CONNECTED);
                        is_connected = true;
                    }
                    buffer_length += static_cast<size_t>(bytes_read);

                    // Process all complete frames in the buffer
                    size_t index = 0;
                    while (buffer_length - index >= TOTAL_FRAME_SIZE) {
                        // Find the next frame start byte
                        while (index < buffer_length && rx_buffer[index] != FRAME_START_BYTE) index++;
                        if (buffer_length - index < TOTAL_FRAME_SIZE) break;

                        // Verify payload length
                        if (rx_buffer[index+1] != PAYLOAD_SIZE_BYTES) {
                            index++; continue;
                        }

                        // Validate checksum
                        uint8_t calculated_checksum = calculateChecksum(&rx_buffer[index], 1 + 1 + PAYLOAD_SIZE_BYTES);
                        uint8_t received_checksum = rx_buffer[index + 1 + 1 + PAYLOAD_SIZE_BYTES];
                        
                        if (calculated_checksum == received_checksum) {
                            processTelemetryPacket(&rx_buffer[index+2]);
                            index += TOTAL_FRAME_SIZE;
                        } else {
                            index++;   // Checksum mismatch, skip one byte and re‑sync
                        }
                    }

                    // Shift remaining data to the front of the buffer
                    if (index > 0) {
                        size_t remaining_bytes = buffer_length - index;
                        if (remaining_bytes) memmove(rx_buffer, rx_buffer + index, remaining_bytes);
                        buffer_length = remaining_bytes;
                    }
                    // If buffer full and no valid frame found, discard everything
                    if (buffer_length > sizeof(rx_buffer) - TOTAL_FRAME_SIZE) buffer_length = 0;
                    
                } else if (bytes_read == 0) {
                    // EOF – remote device closed the connection
                    break;
                } else {
                    // Non fatal read error, retry
                    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) continue;
                    
                    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000, 
                        "Read error on serial port: %s", strerror(errno));
                    break;
                }
            } 
            // Handle errors or hangup
            else if (poll_fd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
                break;
            }
        }

        // Clean up
        int current_fd = serial_file_descriptor_.exchange(-1);
        if (current_fd >= 0) {
            close(current_fd);
        }

        if (is_connected) {
            // Notify disconnection
            publishTeleopEvent(hexapod_custom_msgs::msg::TeleopEvent::BT_DISCONNECTED);
            is_connected = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}