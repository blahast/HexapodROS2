#ifndef HEXAPOD_LOCOMOTION_NODE_HPP
#define HEXAPOD_LOCOMOTION_NODE_HPP

#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>
#include <vector>
#include <string>
#include <array>
#include <algorithm>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <std_msgs/msg/u_int8.hpp>

#include "hexapod_custom_msgs/msg/locomotion_command.hpp"
#include "hexapod_custom_msgs/msg/locomotion_state.hpp"

// Leg geometry (mm)
constexpr double L_COXA = 48.0;
constexpr double L_FEMUR = 75.0;
constexpr double L_TIBIA = 116.0;
constexpr double BASE_RADIUS = 78.0;
constexpr double TIBIA_ANGLE_RAD = 24.9 * M_PI / 180.0;   // Tibia angular offset
constexpr double ALPHA0 = 30.0 * M_PI / 180.0;            // First leg angular offset

// Default leg positions in body frame
constexpr double DEFAULT_DISTANCE = 200.0;
constexpr double DEFAULT_HEIGHT = -80.0;
constexpr double INITIAL_DISTANCE = 172.0;
constexpr double INITIAL_HEIGHT = -20.0;
constexpr double GROUND_HEIGHT = -28.0;

// Input mapping limits from teleop range [-1, 1]
constexpr double MIN_FREQ = 0.1;
constexpr double MAX_FREQ = 1.0;
constexpr double MIN_STEP_H = 0.0;
constexpr double MAX_STEP_H = 100.0;
constexpr double MAX_X_OFF = 70.0;      
constexpr double MAX_Y_OFF = 70.0;      
constexpr double MAX_Z_OFF = 60.0;      
constexpr double MAX_ROLL = 0.3;       
constexpr double MAX_PITCH = 0.3;      
constexpr double MAX_YAW = 0.4;        

// URDF direction signs, affects only RViz visualisation
constexpr double URDF_COXA_SIGN = 1.0;
constexpr double URDF_FEMUR_SIGN = -1.0;
constexpr double URDF_TIBIA_SIGN = 1.0;

constexpr int NUM_OF_LEGS = 6;
constexpr int JOINTS_PER_LEG = 3;

struct Vector3 { double x, y, z; };
struct Vec2 { double x, y; };

// Servo limits, inversion, offset for zero position
struct ServoConfig {
    double min_angle;
    double max_angle;
    bool inverted;
    double angle_offset;
};

struct GaitParams {
    std::array<double, NUM_OF_LEGS> phaseOffsets;
    double beta;                // duty cycle (stance fraction)
    double max_speed;           // max linear speed (mm/s)
    double max_turning_speed;   // max angular speed (rad/s)
};

// Servo configuration
// Min angle, max angle, inverted, angle offset
inline constexpr ServoConfig servo_config[NUM_OF_LEGS][JOINTS_PER_LEG] = {
    {{45.0, 145.0, false, 90.0}, 
    {-20.0, 200.0, true, 90.0},
    {38.0, 200.0, true, 0.0}},

    {{35.0, 145.0, false, 90.0},
    {-20.0, 200.0, true, 90.0},
    {38.0, 200.0, true, 0.0}},

    {{35.0, 135.0, false, 90.0},
    {-20.0, 200.0, true, 90.0},
    {38.0, 200.0, true, 0.0}},

    {{45.0, 145.0, false, 90.0},
    {-20.0, 200.0, false, 90.0},
    {38.0, 200.0, false, 0.0}},

    {{35.0, 145.0, false, 90.0},
    {-20.0, 200.0, false, 90.0},
    {38.0, 200.0, false, 0.0}},

    {{35.0, 135.0, false, 90.0},
    {-20.0, 200.0, false, 90.0},
    {38.0, 200.0, false, 0.0}}
};

// Predefined gait patterns: phase offsets, beta, max_speed, max_turning_speed
inline constexpr GaitParams gaits[] = {
    {{0.0, 0.5, 0.0, 0.5, 0.0, 0.5}, 0.53, 80.0, 0.6},                          // TRIPOD — 3+3 legs
    {{0.0, 1.0/3.0, 2.0/3.0, 0.0, 1.0/3.0, 2.0/3.0}, 2.0/3.0, 60.0, 0.4},       // TETRAPOD — three phases with pairs (0,3) (1,4) (2,5)
    {{0.0, 2.0/6.0, 4.0/6.0, 1.0/6.0, 3.0/6.0, 5.0/6.0}, 5.0/6.0, 40.0, 0.35}   // RIPPLE — one leg at a time
};

// Animation keyframe: leg target positions
struct AnimKeyframe {
    std::array<Vector3, NUM_OF_LEGS> leg_targets;   // Leg target positions
    double duration_s;                              // Duration of the keyframe [s]
    bool ease;                                      // easing type, true: smooth 7‑order interpolation, false: linear
};

class HexapodLocomotionNode : public rclcpp::Node {
public:
    explicit HexapodLocomotionNode(const rclcpp::NodeOptions &options = rclcpp::NodeOptions());
    ~HexapodLocomotionNode() = default;

private:
    enum class State { INIT, SITTING, STANDING, WALKING, ANIMATING };
    State current_state_ = State::SITTING;
    
    rclcpp::TimerBase::SharedPtr timer_;
    std::mutex data_mutex_;

    double loop_rate_hz_;
    double loop_period_s_;

    // Teleop targets
    double target_vx_ = 0.0, target_vy_ = 0.0, target_omega_ = 0.0;
    double target_off_x_ = 0.0, target_off_y_ = 0.0, target_off_z_ = 0.0;
    double target_roll_ = 0.0, target_pitch_ = 0.0, target_yaw_ = 0.0;
    
    // Gait and locomotion parameters
    int target_gait_ = 0;
    double target_freq_ = MIN_FREQ;
    double target_step_h_ = MIN_STEP_H;
    int pending_command_ = -1;   // Locomotion command ID
    
    // Smoothed gait parameters
    double current_beta_;
    std::array<double, NUM_OF_LEGS> current_phase_offsets_;

    // Smoothed body offsets
    double current_off_x_ = 0.0, current_off_y_ = 0.0, current_off_z_ = 0.0;
    double current_roll_ = 0.0, current_pitch_ = 0.0, current_yaw_ = 0.0;

    // Walking dynamics
    double current_freq_ = MIN_FREQ;
    double current_vx_ = 0.0, current_vy_ = 0.0, current_omega_ = 0.0, current_swing_h_ = MIN_STEP_H;
    double phase_base_ = 0.0;
    bool is_walking_ = false;
    bool stopping_ = false;
    bool was_stopping_ = false;

    // Leg geometry and state
    Vector3 leg_anchor_[NUM_OF_LEGS];           // Coxa pivot points in body frame
    double leg_alpha_[NUM_OF_LEGS];             // Angular position of each leg around body
    Vector3 default_leg_pos_[NUM_OF_LEGS];      // Default standing positions
    Vector3 leg_pos_[NUM_OF_LEGS];              // Current target positions
    double target_angles_rad_[NUM_OF_LEGS][JOINTS_PER_LEG];
    int stopping_swings_[NUM_OF_LEGS] = {0};

    // Animation state
    std::vector<AnimKeyframe> anim_sequence_;
    size_t current_anim_frame_ = 0;
    double anim_t_ = 0.0;
    std::array<Vector3, NUM_OF_LEGS> anim_start_pos_;
    State state_after_anim_ = State::STANDING;

    // Swing state for gait generation
    struct LegSwingState {
        bool in_swing = false;
        bool first_tick = false;
        double x_lo = 0.0, y_lo = 0.0;       // Lift‑off position
        double x_td = 0.0, y_td = 0.0;       // Touch‑down position
        int swings_done = 0;
        double tau0 = 0.0;                   // Phase at lift‑off
    } swing_state_[NUM_OF_LEGS];

    // ROS publishers/subscribers
    rclcpp::Publisher<hexapod_custom_msgs::msg::LocomotionState>::SharedPtr status_pub_;
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr hw_angles_pub_;
    
    rclcpp::Subscription<hexapod_custom_msgs::msg::LocomotionCommand>::SharedPtr cmd_sub_;
    rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr gait_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr vel_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Pose>::SharedPtr pose_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr params_sub_;

    // Callbacks
    void cmdCallback(const hexapod_custom_msgs::msg::LocomotionCommand::SharedPtr msg);
    void gaitCallback(const std_msgs::msg::UInt8::SharedPtr msg);
    void velCallback(const geometry_msgs::msg::Twist::SharedPtr msg);
    void poseCallback(const geometry_msgs::msg::Pose::SharedPtr msg);
    void paramsCallback(const std_msgs::msg::Float32MultiArray::SharedPtr msg);
    
    void publishState(State s);
    void locomotionLoopStep(); // Timer callback
    
    // Animation builders
    void startAnimation(const std::vector<AnimKeyframe>& sequence, State next_state);
    void buildStandUpAnimation();
    void buildSitDownAnimation();
    void buildWaveAnimation();

    // Gait and kinematics helpers
    void generateStepPoint(int leg, double phase, double dt, double beta, double T, 
                           double omega, double vx, double vy, double h, double swing_h, bool stopping);
    void rotateNegRPYAndTranslate(const Vector3& p, const Vector3& off_xyz, const Vector3& off_rpy, 
                                  double &x_ee, double &y_ee, double &z_ee);
    void inverseKinematics(const Vector3& off_xyz, const Vector3& off_rpy);
    void publishAngles();
};

#endif // HEXAPOD_LOCOMOTION_NODE_HPP