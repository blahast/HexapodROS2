cmake_minimum_required(VERSION 3.8)
project(hexapod_hardware)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(hexapod_custom_msgs REQUIRED)
find_library(WIRINGPI_LIBRARIES NAMES wiringPi)

include_directories(include)

add_library(buzzer_driver_node SHARED
  src/buzzer_driver_node.cpp
)

target_include_directories(buzzer_driver_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(buzzer_driver_node rclcpp std_msgs hexapod_custom_msgs)

if(WIRINGPI_LIBRARIES)
  target_link_libraries(buzzer_driver_node ${WIRINGPI_LIBRARIES})
else()
  message(FATAL_ERROR "wiringPi library not found. Please install it (e.g., sudo apt install wiringpi).")
endif()

ament_export_include_directories(include)
ament_export_libraries(buzzer_driver_node)
ament_export_dependencies(rclcpp std_msgs hexapod_custom_msgs)

add_executable(buzzer_driver_node_standalone src/main.cpp)

target_link_libraries(buzzer_driver_node_standalone buzzer_driver_node)
ament_target_dependencies(buzzer_driver_node_standalone rclcpp hexapod_custom_msgs)

install(TARGETS buzzer_driver_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(TARGETS buzzer_driver_node_standalone
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()