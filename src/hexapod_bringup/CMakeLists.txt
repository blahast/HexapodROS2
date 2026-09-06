cmake_minimum_required(VERSION 3.8)
project(hexapod_bringup)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(hexapod_custom_msgs REQUIRED)
find_package(hexapod_hardware REQUIRED)
find_package(hexapod_control REQUIRED)
find_package(hexapod_teleop REQUIRED)

add_executable(hexapod_shared_process src/main.cpp)

ament_target_dependencies(hexapod_shared_process
  rclcpp
  hexapod_custom_msgs
  hexapod_hardware
  hexapod_control
  hexapod_teleop
)

install(TARGETS
  hexapod_shared_process
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()