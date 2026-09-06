// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from hexapod_custom_msgs:msg/LocomotionCommand.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "hexapod_custom_msgs/msg/locomotion_command.h"


#ifndef HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_COMMAND__STRUCT_H_
#define HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_COMMAND__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

/// Constant 'CMD_STAND_UP'.
enum
{
  hexapod_custom_msgs__msg__LocomotionCommand__CMD_STAND_UP = 1
};

/// Constant 'CMD_SIT_DOWN'.
enum
{
  hexapod_custom_msgs__msg__LocomotionCommand__CMD_SIT_DOWN = 2
};

/// Constant 'CMD_CHANGE_GAIT'.
enum
{
  hexapod_custom_msgs__msg__LocomotionCommand__CMD_CHANGE_GAIT = 3
};

/// Constant 'CMD_PLAY_ANIMATION'.
enum
{
  hexapod_custom_msgs__msg__LocomotionCommand__CMD_PLAY_ANIMATION = 4
};

/// Struct defined in msg/LocomotionCommand in the package hexapod_custom_msgs.
typedef struct hexapod_custom_msgs__msg__LocomotionCommand
{
  uint8_t command_id;
} hexapod_custom_msgs__msg__LocomotionCommand;

// Struct for a sequence of hexapod_custom_msgs__msg__LocomotionCommand.
typedef struct hexapod_custom_msgs__msg__LocomotionCommand__Sequence
{
  hexapod_custom_msgs__msg__LocomotionCommand * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} hexapod_custom_msgs__msg__LocomotionCommand__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_COMMAND__STRUCT_H_
