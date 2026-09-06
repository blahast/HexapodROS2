// generated from rosidl_typesupport_fastrtps_c/resource/idl__rosidl_typesupport_fastrtps_c.h.em
// with input from hexapod_custom_msgs:msg/BuzzerCommand.idl
// generated code does not contain a copyright notice
#ifndef HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_
#define HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_


#include <stddef.h>
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_interface/macros.h"
#include "hexapod_custom_msgs/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "hexapod_custom_msgs/msg/detail/buzzer_command__struct.h"
#include "fastcdr/Cdr.h"

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_hexapod_custom_msgs
bool cdr_serialize_hexapod_custom_msgs__msg__BuzzerCommand(
  const hexapod_custom_msgs__msg__BuzzerCommand * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_hexapod_custom_msgs
bool cdr_deserialize_hexapod_custom_msgs__msg__BuzzerCommand(
  eprosima::fastcdr::Cdr &,
  hexapod_custom_msgs__msg__BuzzerCommand * ros_message);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_hexapod_custom_msgs
size_t get_serialized_size_hexapod_custom_msgs__msg__BuzzerCommand(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_hexapod_custom_msgs
size_t max_serialized_size_hexapod_custom_msgs__msg__BuzzerCommand(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_hexapod_custom_msgs
bool cdr_serialize_key_hexapod_custom_msgs__msg__BuzzerCommand(
  const hexapod_custom_msgs__msg__BuzzerCommand * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_hexapod_custom_msgs
size_t get_serialized_size_key_hexapod_custom_msgs__msg__BuzzerCommand(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_hexapod_custom_msgs
size_t max_serialized_size_key_hexapod_custom_msgs__msg__BuzzerCommand(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_hexapod_custom_msgs
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, hexapod_custom_msgs, msg, BuzzerCommand)();

#ifdef __cplusplus
}
#endif

#endif  // HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_
