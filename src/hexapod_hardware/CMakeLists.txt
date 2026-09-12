cmake_minimum_required(VERSION 3.8)
project(hexapod_hardware)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(hexapod_custom_msgs REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED IMPORTED_TARGET libgpiodcxx)

include_directories(include)

add_library(buzzer_driver_node SHARED
  src/buzzer_driver_node.cpp
)

target_include_directories(buzzer_driver_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(buzzer_driver_node rclcpp hexapod_custom_msgs)
target_link_libraries(buzzer_driver_node PkgConfig::GPIOD)

add_library(servo_driver_node SHARED
  src/servo_driver_node.cpp
)

target_include_directories(servo_driver_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(servo_driver_node rclcpp std_msgs)
target_link_libraries(servo_driver_node PkgConfig::GPIOD)

add_library(switch_node SHARED
  src/switch_node.cpp
)

target_include_directories(switch_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(switch_node rclcpp hexapod_custom_msgs)
target_link_libraries(switch_node PkgConfig::GPIOD)

add_library(imu_node SHARED
  src/imu_node.cpp
)

target_include_directories(imu_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(imu_node rclcpp sensor_msgs)
target_link_libraries(imu_node)

ament_export_include_directories(include)
ament_export_libraries(buzzer_driver_node servo_driver_node switch_node imu_node)
ament_export_dependencies(rclcpp std_msgs sensor_msgs hexapod_custom_msgs)

add_executable(buzzer_driver_node_standalone src/buzzer_main.cpp)
target_link_libraries(buzzer_driver_node_standalone buzzer_driver_node)
ament_target_dependencies(buzzer_driver_node_standalone rclcpp hexapod_custom_msgs)

add_executable(servo_driver_node_standalone src/servo_main.cpp)
target_link_libraries(servo_driver_node_standalone servo_driver_node)
ament_target_dependencies(servo_driver_node_standalone rclcpp std_msgs)

add_executable(switch_node_standalone src/switch_main.cpp)
target_link_libraries(switch_node_standalone switch_node)
ament_target_dependencies(switch_node_standalone rclcpp hexapod_custom_msgs)

add_executable(imu_node_standalone src/imu_main.cpp)
target_link_libraries(imu_node_standalone imu_node)
ament_target_dependencies(imu_node_standalone rclcpp sensor_msgs)

install(TARGETS
  buzzer_driver_node
  servo_driver_node
  switch_node
  imu_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(TARGETS
  buzzer_driver_node_standalone
  servo_driver_node_standalone
  switch_node_standalone
  imu_node_standalone
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()