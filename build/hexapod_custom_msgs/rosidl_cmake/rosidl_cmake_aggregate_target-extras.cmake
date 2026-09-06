# generated from rosidl_cmake/cmake/rosidl_cmake_aggregate_target-extras.cmake.in

# Create a convenience aggregate target hexapod_custom_msgs::hexapod_custom_msgs
# that links all generated interface targets, so downstream packages can use
# a single modern CMake target name instead of ${hexapod_custom_msgs_TARGETS}.
if(hexapod_custom_msgs_TARGETS AND NOT TARGET hexapod_custom_msgs::hexapod_custom_msgs)
  add_library(hexapod_custom_msgs::hexapod_custom_msgs INTERFACE IMPORTED)
  set_target_properties(hexapod_custom_msgs::hexapod_custom_msgs PROPERTIES
    INTERFACE_LINK_LIBRARIES "${hexapod_custom_msgs_TARGETS}")
endif()
