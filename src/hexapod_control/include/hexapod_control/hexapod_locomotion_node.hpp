#ifndef HEXAPOD_LOCOMOTION_NODE_HPP
#define HEXAPOD_LOCOMOTION_NODE_HPP

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>

#include "hexapod_custom_msgs/msg/locomotion_command.hpp"
#include "hexapod_custom_msgs/msg/locomotion_state.hpp"

#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>
#include <vector>
#include <string>
#include <array>
#include <algorithm>

// =============================================================================
// KONSTANTY, LIMITY A STRUKTURY
// =============================================================================

constexpr double L_COXA = 48.0;
constexpr double L_FEMUR = 75.0;
constexpr double L_TIBIA = 116.0;
constexpr double BASE_RADIUS = 78.0;
constexpr double TIBIA_ANGLE_RAD = 24.9 * M_PI / 180.0;
constexpr double ALPHA0 = 30.0 * M_PI / 180.0;

constexpr double DEFAULT_DISTANCE = 200.0;
constexpr double DEFAULT_HEIGHT = -80.0;
constexpr double INITIAL_DISTANCE = 172.0;
constexpr double INITIAL_HEIGHT = -20.0;
constexpr double GROUND_HEIGHT = -28.0;

// Limity pro mapovani vstupu (z teleopu chodi -1 az 1)
constexpr double MIN_FREQ = 0.1;        constexpr double MAX_FREQ = 1.0;
constexpr double MIN_STEP_H = 0.0;     constexpr double MAX_STEP_H = 100.0;
constexpr double MAX_X_OFF = 70.0;      
constexpr double MAX_Y_OFF = 70.0;      
constexpr double MAX_Z_OFF = 60.0;      
constexpr double MAX_ROLL = 0.3;       
constexpr double MAX_PITCH = 0.3;      
constexpr double MAX_YAW = 0.4;        

// URDF Invertory (Srovnani vizualizace v RViz bez vlivu na realny HW)
// Pokud je noha porad vyvracena, zmen znamenko na 1.0 nebo -1.0
constexpr double URDF_COXA_SIGN = 1.0;
constexpr double URDF_FEMUR_SIGN = -1.0;
constexpr double URDF_TIBIA_SIGN = 1.0;

constexpr int NUM_OF_LEGS = 6;
constexpr int JOINTS_PER_LEG = 3;

struct Vector3 { double x, y, z; };
struct Vec2 { double x, y; };

struct LegPosition {
    Vector3 current;
    Vector3 target;
};

struct ServoConfig {
    double min_angle;
    double max_angle;
    bool inverted;
    double angle_offset;
};

struct GaitParams {
    std::array<double, NUM_OF_LEGS> phaseOffsets;
    double beta;
    double max_speed;
    double max_turning_speed;
};

// C++17 inline definice primo v hlavicce
inline constexpr ServoConfig servo_config[NUM_OF_LEGS][JOINTS_PER_LEG] = {
    {{45.0, 145.0, false, 90.0}, {-20.0, 200.0, true, 90.0}, {38.0, 200.0, true, 0.0}},
    {{35.0, 145.0, false, 90.0}, {-20.0, 200.0, true, 90.0}, {38.0, 200.0, true, 0.0}},
    {{35.0, 135.0, false, 90.0}, {-20.0, 200.0, true, 90.0}, {38.0, 200.0, true, 0.0}},
    {{45.0, 145.0, false, 90.0}, {-20.0, 200.0, false, 90.0}, {38.0, 200.0, false, 0.0}},
    {{35.0, 145.0, false, 90.0}, {-20.0, 200.0, false, 90.0}, {38.0, 200.0, false, 0.0}},
    {{35.0, 135.0, false, 90.0}, {-20.0, 200.0, false, 90.0}, {38.0, 200.0, false, 0.0}}
};

inline constexpr GaitParams gaits[] = {
    {{0.0, 0.5, 0.0, 0.5, 0.0, 0.5}, 0.53, 80.0, 0.6},                     // TRIPOD
    {{0.0, 1.0/3.0, 2.0/3.0, 0.0, 1.0/3.0, 2.0/3.0}, 2.0/3.0, 60.0, 0.4},  // TETRAPOD
    {{0.0, 2.0/6.0, 4.0/6.0, 1.0/6.0, 3.0/6.0, 5.0/6.0}, 5.0/6.0, 40.0, 0.35} // RIPPLE
};

// =============================================================================
// TŘÍDA UZLU
// =============================================================================

class HexapodLocomotionNode : public rclcpp::Node {
public:
    explicit HexapodLocomotionNode(const rclcpp::NodeOptions &options = rclcpp::NodeOptions());
    ~HexapodLocomotionNode();

private:
    enum class State { INIT, SITTING, STANDING, WALKING, ANIMATING };
    State current_state_ = State::SITTING;
    
    std::atomic<bool> run_thread_;
    std::thread locomotion_thread_;
    std::mutex data_mutex_;

    double loop_rate_hz_;
    double loop_period_s_;

    // Teleop data (cílové namapované stavy)
    double target_vx_ = 0.0, target_vy_ = 0.0, target_omega_ = 0.0;
    double target_off_x_ = 0.0, target_off_y_ = 0.0, target_off_z_ = 0.0;
    double target_roll_ = 0.0, target_pitch_ = 0.0, target_yaw_ = 0.0;
    
    // Walking parametry
    int current_gait_ = 0;
    double target_freq_ = MIN_FREQ;
    double target_step_h_ = MIN_STEP_H;
    int pending_command_ = -1; 

    // Aktuální offsety těla (udržováno pro plynulé navrácení do nuly před animací)
    double current_off_x_ = 0.0, current_off_y_ = 0.0, current_off_z_ = 0.0;
    double current_roll_ = 0.0, current_pitch_ = 0.0, current_yaw_ = 0.0;

    Vector3 leg_anchor_[NUM_OF_LEGS];
    double leg_alpha_[NUM_OF_LEGS];
    Vector3 default_leg_pos_[NUM_OF_LEGS];
    LegPosition leg_pos_[NUM_OF_LEGS];
    
    double target_angles_rad_[NUM_OF_LEGS][JOINTS_PER_LEG];
    int stopping_swings_[NUM_OF_LEGS] = {0};

    struct LegSwingState {
        bool in_swing = false;
        bool first_tick = false;
        double x_lo = 0.0, y_lo = 0.0;
        double x_td = 0.0, y_td = 0.0;
        int swings_done = 0;
        double tau0 = 0.0;
    } swing_state_[NUM_OF_LEGS];

    // Subscribery & Publishery
    rclcpp::Publisher<hexapod_custom_msgs::msg::LocomotionState>::SharedPtr status_pub_;
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr hw_angles_pub_;
    
    rclcpp::Subscription<hexapod_custom_msgs::msg::LocomotionCommand>::SharedPtr cmd_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr vel_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Pose>::SharedPtr pose_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32MultiArray>::SharedPtr params_sub_;

    // Metody
    void cmdCallback(const hexapod_custom_msgs::msg::LocomotionCommand::SharedPtr msg);
    void velCallback(const geometry_msgs::msg::Twist::SharedPtr msg);
    void poseCallback(const geometry_msgs::msg::Pose::SharedPtr msg);
    void paramsCallback(const std_msgs::msg::Float32MultiArray::SharedPtr msg);
    
    void publishState(State s);
    void locomotionLoop();
    void generateStepPoint(int leg, double phase, double dt, double beta, double T, 
                           double omega, double vx, double vy, double h, double swing_h, bool stopping);
    void rotateNegRPYAndTranslate(const Vector3& p, const Vector3& off_xyz, const Vector3& off_rpy, 
                                  double &x_ee, double &y_ee, double &z_ee);
    void inverseKinematics(const Vector3& off_xyz, const Vector3& off_rpy);
    void publishAngles();

    void interpolateOffsetsToZero(double duration);
    void interpolateLegs(const Vector3 end_targets[NUM_OF_LEGS], double duration);
    void standUpAnimation();
    void sitDownAnimation();
    void waveAnimation();
};

#endif // HEXAPOD_LOCOMOTION_NODE_HPP