// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from hexapod_custom_msgs:msg/LocomotionState.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "hexapod_custom_msgs/msg/locomotion_state.h"


#ifndef HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__STRUCT_H_
#define HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

/// Constant 'INIT'.
enum
{
  hexapod_custom_msgs__msg__LocomotionState__INIT = 0
};

/// Constant 'SITTING'.
enum
{
  hexapod_custom_msgs__msg__LocomotionState__SITTING = 1
};

/// Constant 'STANDING'.
enum
{
  hexapod_custom_msgs__msg__LocomotionState__STANDING = 2
};

/// Constant 'WALKING'.
enum
{
  hexapod_custom_msgs__msg__LocomotionState__WALKING = 3
};

/// Constant 'ANIMATING'.
enum
{
  hexapod_custom_msgs__msg__LocomotionState__ANIMATING = 4
};

/// Struct defined in msg/LocomotionState in the package hexapod_custom_msgs.
typedef struct hexapod_custom_msgs__msg__LocomotionState
{
  uint8_t state_id;
} hexapod_custom_msgs__msg__LocomotionState;

// Struct for a sequence of hexapod_custom_msgs__msg__LocomotionState.
typedef struct hexapod_custom_msgs__msg__LocomotionState__Sequence
{
  hexapod_custom_msgs__msg__LocomotionState * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} hexapod_custom_msgs__msg__LocomotionState__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__STRUCT_H_
