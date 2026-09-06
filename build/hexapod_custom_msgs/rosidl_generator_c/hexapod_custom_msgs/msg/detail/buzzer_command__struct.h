// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from hexapod_custom_msgs:msg/BuzzerCommand.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "hexapod_custom_msgs/msg/buzzer_command.h"


#ifndef HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__STRUCT_H_
#define HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

/// Constant 'BEEP'.
enum
{
  hexapod_custom_msgs__msg__BuzzerCommand__BEEP = 1
};

/// Constant 'STARTUP'.
enum
{
  hexapod_custom_msgs__msg__BuzzerCommand__STARTUP = 2
};

/// Constant 'SHUTDOWN'.
enum
{
  hexapod_custom_msgs__msg__BuzzerCommand__SHUTDOWN = 3
};

/// Constant 'ERROR'.
enum
{
  hexapod_custom_msgs__msg__BuzzerCommand__ERROR = 4
};

/// Constant 'MELODY'.
enum
{
  hexapod_custom_msgs__msg__BuzzerCommand__MELODY = 5
};

/// Constant 'BT_CONNECTED'.
enum
{
  hexapod_custom_msgs__msg__BuzzerCommand__BT_CONNECTED = 6
};

/// Constant 'BT_DISCONNECTED'.
enum
{
  hexapod_custom_msgs__msg__BuzzerCommand__BT_DISCONNECTED = 7
};

/// Struct defined in msg/BuzzerCommand in the package hexapod_custom_msgs.
typedef struct hexapod_custom_msgs__msg__BuzzerCommand
{
  uint8_t command_id;
} hexapod_custom_msgs__msg__BuzzerCommand;

// Struct for a sequence of hexapod_custom_msgs__msg__BuzzerCommand.
typedef struct hexapod_custom_msgs__msg__BuzzerCommand__Sequence
{
  hexapod_custom_msgs__msg__BuzzerCommand * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} hexapod_custom_msgs__msg__BuzzerCommand__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__STRUCT_H_
