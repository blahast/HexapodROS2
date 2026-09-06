cmake_minimum_required(VERSION 3.8)
project(hexapod_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(hexapod_custom_msgs REQUIRED)

# 1. Knihovna pro sdílený proces
add_library(hexapod_brain_node SHARED
  src/hexapod_brain_node.cpp
)

target_include_directories(hexapod_brain_node
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(hexapod_brain_node
  rclcpp
  geometry_msgs
  std_msgs
  hexapod_custom_msgs
)

ament_export_include_directories(include)
ament_export_libraries(hexapod_brain_node)
ament_export_dependencies(
  rclcpp
  geometry_msgs
  std_msgs
  hexapod_custom_msgs
)

# 2. Samostatně spustitelný uzel
add_executable(hexapod_brain_node_standalone
  src/main.cpp
)

target_link_libraries(hexapod_brain_node_standalone
  hexapod_brain_node
)

ament_target_dependencies(hexapod_brain_node_standalone
  rclcpp
  geometry_msgs
  std_msgs
  hexapod_custom_msgs
)

# 3. Instalace
install(TARGETS hexapod_brain_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(TARGETS hexapod_brain_node_standalone
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

ament_package()