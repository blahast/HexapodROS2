cmake_minimum_required(VERSION 3.8)
project(hexapod_teleop)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(hexapod_custom_msgs REQUIRED)

add_library(bluetooth_teleop_node SHARED
  src/bluetooth_teleop_node.cpp
)

target_include_directories(bluetooth_teleop_node
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(bluetooth_teleop_node rclcpp geometry_msgs std_msgs tf2 hexapod_custom_msgs)

ament_export_include_directories(include)
ament_export_libraries(bluetooth_teleop_node)
ament_export_dependencies(rclcpp geometry_msgs std_msgs tf2 hexapod_custom_msgs)

add_executable(bluetooth_teleop_node_standalone src/main.cpp)

target_link_libraries(bluetooth_teleop_node_standalone bluetooth_teleop_node)
ament_target_dependencies(bluetooth_teleop_node_standalone rclcpp hexapod_custom_msgs)

install(TARGETS bluetooth_teleop_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(TARGETS bluetooth_teleop_node_standalone
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()