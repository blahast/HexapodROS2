cmake_minimum_required(VERSION 3.8)
project(hexapod_custom_msgs)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/TeleopEvent.msg"
  "msg/BuzzerCommand.msg"
  "msg/LocomotionState.msg"
  "msg/LocomotionCommand.msg"
  "msg/GaitCommand.msg"
)

ament_package()