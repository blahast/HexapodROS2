// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from hexapod_custom_msgs:msg/TeleopEvent.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "hexapod_custom_msgs/msg/teleop_event.h"


#ifndef HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__TELEOP_EVENT__STRUCT_H_
#define HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__TELEOP_EVENT__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

/// Constant 'BT_CONNECTED'.
enum
{
  hexapod_custom_msgs__msg__TeleopEvent__BT_CONNECTED = 1
};

/// Constant 'BT_DISCONNECTED'.
enum
{
  hexapod_custom_msgs__msg__TeleopEvent__BT_DISCONNECTED = 2
};

/// Constant 'TOGGLE_STAND'.
enum
{
  hexapod_custom_msgs__msg__TeleopEvent__TOGGLE_STAND = 3
};

/// Constant 'CHANGE_GAIT'.
enum
{
  hexapod_custom_msgs__msg__TeleopEvent__CHANGE_GAIT = 4
};

/// Constant 'PLAY_ANIMATION'.
enum
{
  hexapod_custom_msgs__msg__TeleopEvent__PLAY_ANIMATION = 5
};

/// Struct defined in msg/TeleopEvent in the package hexapod_custom_msgs.
typedef struct hexapod_custom_msgs__msg__TeleopEvent
{
  uint8_t event_id;
} hexapod_custom_msgs__msg__TeleopEvent;

// Struct for a sequence of hexapod_custom_msgs__msg__TeleopEvent.
typedef struct hexapod_custom_msgs__msg__TeleopEvent__Sequence
{
  hexapod_custom_msgs__msg__TeleopEvent * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} hexapod_custom_msgs__msg__TeleopEvent__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__TELEOP_EVENT__STRUCT_H_
