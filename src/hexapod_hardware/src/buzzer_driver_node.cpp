#include <wiringPi.h>
#include <softTone.h>

#include "hexapod_hardware/buzzer_driver_node.hpp"
#include "hexapod_hardware/notes.hpp"
#include "hexapod_hardware/buzzer_driver_node.hpp"

BuzzerDriverNode::BuzzerDriverNode(const rclcpp::NodeOptions & options) 
    : Node("buzzer_driver", options) {
    // Read GPIO pin parameter (default 0)
    this->declare_parameter<int>("buzzer_pin", 0);
    buzzer_pin_ = this->get_parameter("buzzer_pin").as_int();

    // Initialize WiringPi library for GPIO access
    if (wiringPiSetup() == -1) {
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize wiringPi.");
    }
    
    // Configure pin
    if (softToneCreate(buzzer_pin_) != 0) {
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize softTone on pin %d", buzzer_pin_);
    }

    // Subscriber for buzzer commands
    command_subscriber_ = this->create_subscription<hexapod_custom_msgs::msg::BuzzerCommand>(
        "play_melody", 10,
        [this](const hexapod_custom_msgs::msg::BuzzerCommand::SharedPtr msg) {
            commandCallback(msg);
        }
    );

    RCLCPP_INFO(this->get_logger(), "Buzzer driver node initialized on pin %d.", buzzer_pin_);

    // Start thread for playing melodies
    is_running_ = true;
    play_thread_ = std::thread(&BuzzerDriverNode::playLoop, this);

    // Play startup melody
    hexapod_custom_msgs::msg::BuzzerCommand startup_msg;
    startup_msg.command_id = hexapod_custom_msgs::msg::BuzzerCommand::STARTUP;
    commandCallback(std::make_shared<hexapod_custom_msgs::msg::BuzzerCommand>(startup_msg));
}

BuzzerDriverNode::~BuzzerDriverNode() {
    // Signal thread to exit and wake it up if waiting
    is_running_ = false;
    queue_cv_.notify_all();

    if (play_thread_.joinable()) {
        play_thread_.join();
    }
    
    // Turn off buzzer on shutdown
    softToneWrite(buzzer_pin_, 0);
}

void BuzzerDriverNode::commandCallback(const hexapod_custom_msgs::msg::BuzzerCommand::SharedPtr msg) {
    // Push command to queue
    std::lock_guard<std::mutex> lock(queue_mutex_);
    melody_queue_.push(msg->command_id);
    queue_cv_.notify_one();
}

void BuzzerDriverNode::playLoop() {
    using Cmd = hexapod_custom_msgs::msg::BuzzerCommand;
    while (is_running_) {
        uint8_t current_command = 0;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            // Wait until queue has data or shutdown requested
            queue_cv_.wait(lock, [this]() { return !melody_queue_.empty() || !is_running_; });
            if (!is_running_ && melody_queue_.empty()) break;
            
            current_command = melody_queue_.front();
            melody_queue_.pop();
        }

        // Play melody
        switch(current_command) {
            case Cmd::BEEP: playBeep(); break;
            case Cmd::STARTUP: playStartupMelody(); break;
            case Cmd::SHUTDOWN: playShutdownMelody(); break;
            case Cmd::ERROR: playErrorMelody(); break;
            case Cmd::MELODY: playMelody(); break;
            case Cmd::BT_CONNECTED: playBtConnectedMelody(); break;
            case Cmd::BT_DISCONNECTED: playBtDisconnectedMelody(); break;
            default: RCLCPP_WARN(this->get_logger(), "Neznamy kod melodie: %d", current_command);
        }
    }
}

// Low-level tone player
void BuzzerDriverNode::playTone(int freq_hz, int duration_ms) {
    if (freq_hz <= 0) {
        softToneWrite(buzzer_pin_, 0);
        delay(duration_ms); // from wiringPi
        return;
    }
    softToneWrite(buzzer_pin_, freq_hz);
    delay(duration_ms);
    softToneWrite(buzzer_pin_, 0);
    delay(5);
}

// --- Predefined melody sequences ---

void BuzzerDriverNode::playBeep() {
    playTone(2000, 80);
    delay(40);
    playTone(2000, 80);
}

void BuzzerDriverNode::playStartupMelody() {
    int notes[] = {NOTE_C5, NOTE_E5, NOTE_G5};
    int durations[] = {150, 150, 300};
    int n = sizeof(notes) / sizeof(notes[0]);
    for (int i = 0; i < n; i++) playTone(notes[i], durations[i]);
}

void BuzzerDriverNode::playShutdownMelody() {
    int notes[] = {NOTE_B5, NOTE_F5, NOTE_E5};
    int durations[] = {150, 150, 300};
    int n = sizeof(notes) / sizeof(notes[0]);
    for (int i = 0; i < n; i++) playTone(notes[i], durations[i]);
}

void BuzzerDriverNode::playErrorMelody() {
    int notes[] = {NOTE_B5, NOTE_F5, NOTE_E5};
    int durations[] = {150, 150, 300};
    int n = sizeof(notes) / sizeof(notes[0]);
    for (int i = 0; i < n; i++) playTone(notes[i], durations[i]);
}

void BuzzerDriverNode::playMelody() {
    const int notes[] = { NOTE_E5, NOTE_D5, NOTE_C5, NOTE_D5, NOTE_E5, NOTE_E5, NOTE_E5, 0, NOTE_D5, NOTE_D5, NOTE_D5, 0, NOTE_E5, NOTE_G5, NOTE_G5, 0 };
    const int durations[] = { 150,150,150,150,150,150,300,100,150,150,300,100,150,150,300,200 };
    const int n = sizeof(notes)/sizeof(notes[0]);
    for (int i = 0; i < n; ++i) playTone(notes[i], durations[i]);
}

void BuzzerDriverNode::playBtConnectedMelody() {
    int notes[] = {NOTE_G5, NOTE_E6};
    int durations[] = {80, 200};
    int n = sizeof(notes) / sizeof(notes[0]);
    for (int i = 0; i < n; i++) playTone(notes[i], durations[i]);
}

void BuzzerDriverNode::playBtDisconnectedMelody() {
    int notes[] = {NOTE_E6, NOTE_G5};
    int durations[] = {80, 200};
    int n = sizeof(notes) / sizeof(notes[0]);
    for (int i = 0; i < n; i++) playTone(notes[i], durations[i]);
}