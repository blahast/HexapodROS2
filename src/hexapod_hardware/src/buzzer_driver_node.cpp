#include <chrono>

#include "hexapod_hardware/buzzer_driver_node.hpp"
#include "hexapod_hardware/notes.hpp"

BuzzerDriverNode::BuzzerDriverNode(const rclcpp::NodeOptions & options) 
    : Node("buzzer_driver", options) {
    
    // Raspberry Pi 5 uses gpiochip4 for the main header pins
    this->declare_parameter<std::string>("buzzer_chip", DEFAULT_GPIO_CHIP_BUZZER);
    buzzer_chip_name_ = this->get_parameter("buzzer_chip").as_string();

    this->declare_parameter<int>("buzzer_pin", DEFAULT_BUZZER_PIN);
    buzzer_pin_ = this->get_parameter("buzzer_pin").as_int();

    // Initialize libgpiod for GPIO access
    try {
        gpio_chip_ = std::make_unique<gpiod::chip>(buzzer_chip_name_);
        buzzer_line_ = gpio_chip_->get_line(buzzer_pin_);
        
        // Configure pin for output
        buzzer_line_.request({"buzzer_driver", gpiod::line_request::DIRECTION_OUTPUT, 0}, 0);
    } catch (const std::exception& e) {
        RCLCPP_ERROR(this->get_logger(), "Failed to initialize gpiod on %s pin %d: %s", 
                     buzzer_chip_name_.c_str(), buzzer_pin_, e.what());
    }

    // Subscribe to buzzer command messages
    command_subscriber_ = this->create_subscription<hexapod_custom_msgs::msg::BuzzerCommand>(
        "play_melody", 10,
        [this](const hexapod_custom_msgs::msg::BuzzerCommand::SharedPtr msg) { commandCallback(msg); });

    RCLCPP_INFO(this->get_logger(), "Buzzer driver node initialized on %s pin %d.", 
                buzzer_chip_name_.c_str(), buzzer_pin_);

    is_running_ = true;
    play_thread_ = std::thread(&BuzzerDriverNode::playLoop, this);

    // Play startup melody
    auto startup_msg = std::make_shared<hexapod_custom_msgs::msg::BuzzerCommand>();
    startup_msg->command_id = hexapod_custom_msgs::msg::BuzzerCommand::STARTUP;
    commandCallback(startup_msg);
}

BuzzerDriverNode::~BuzzerDriverNode() {
    is_running_ = false;
    queue_cv_.notify_all();

    if (play_thread_.joinable()) {
        play_thread_.join();
    }
    
    if (buzzer_line_) {
        buzzer_line_.set_value(0);
        buzzer_line_.release();
    }
}

void BuzzerDriverNode::commandCallback(const hexapod_custom_msgs::msg::BuzzerCommand::SharedPtr msg) {
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
            queue_cv_.wait(lock, [this]() { return !melody_queue_.empty() || !is_running_; });
            if (!is_running_ && melody_queue_.empty()) break;
            
            current_command = melody_queue_.front();
            melody_queue_.pop();
        }

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

// Low-level tone player with precise timing
void BuzzerDriverNode::playTone(int freq_hz, int duration_ms) {
    if (!buzzer_line_) return;

    if (freq_hz <= 0) {
        buzzer_line_.set_value(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
        return;
    }

    int half_period_us = 1000000 / (freq_hz * 2);
    auto start_time = std::chrono::steady_clock::now();
    auto end_time = start_time + std::chrono::milliseconds(duration_ms);

    // Manual software PWM loop using busy-wait for accurate microsecond timing
    while (std::chrono::steady_clock::now() < end_time && is_running_) {
        buzzer_line_.set_value(1);
        auto phase_end = std::chrono::steady_clock::now() + std::chrono::microseconds(half_period_us);
        while(std::chrono::steady_clock::now() < phase_end) {} // Spin lock pro přesný delay

        buzzer_line_.set_value(0);
        phase_end = std::chrono::steady_clock::now() + std::chrono::microseconds(half_period_us);
        while(std::chrono::steady_clock::now() < phase_end) {} // Spin lock pro přesný delay
    }

    buzzer_line_.set_value(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

// --- Predefined melody sequences ---

void BuzzerDriverNode::playBeep() {
    playTone(2000, 80);
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
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