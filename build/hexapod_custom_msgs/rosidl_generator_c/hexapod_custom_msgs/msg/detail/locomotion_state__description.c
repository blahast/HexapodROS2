// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from hexapod_custom_msgs:msg/LocomotionState.idl
// generated code does not contain a copyright notice

#include "hexapod_custom_msgs/msg/detail/locomotion_state__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_hexapod_custom_msgs
const rosidl_type_hash_t *
hexapod_custom_msgs__msg__LocomotionState__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0xeb, 0xf6, 0x75, 0xb7, 0x63, 0xcc, 0x42, 0xba,
      0x36, 0x2d, 0x8b, 0x57, 0xe5, 0x86, 0x5c, 0xe1,
      0xf1, 0xb8, 0x69, 0xda, 0xd9, 0x44, 0x74, 0xa8,
      0x52, 0x82, 0xfa, 0xce, 0x6d, 0xaf, 0xe8, 0x89,
    }};
  return &hash;
}

#include <assert.h>
#include <string.h>

// Include directives for referenced types

// Hashes for external referenced types
#ifndef NDEBUG
#endif

static char hexapod_custom_msgs__msg__LocomotionState__TYPE_NAME[] = "hexapod_custom_msgs/msg/LocomotionState";

// Define type names, field names, and default values
static char hexapod_custom_msgs__msg__LocomotionState__FIELD_NAME__state_id[] = "state_id";

static rosidl_runtime_c__type_description__Field hexapod_custom_msgs__msg__LocomotionState__FIELDS[] = {
  {
    {hexapod_custom_msgs__msg__LocomotionState__FIELD_NAME__state_id, 8, 8},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT8,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
hexapod_custom_msgs__msg__LocomotionState__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {hexapod_custom_msgs__msg__LocomotionState__TYPE_NAME, 39, 39},
      {hexapod_custom_msgs__msg__LocomotionState__FIELDS, 1, 1},
    },
    {NULL, 0, 0},
  };
  if (!constructed) {
    constructed = true;
  }
  return &description;
}

static char toplevel_type_raw_source[] =
  "uint8 INIT=0\n"
  "uint8 SITTING=1\n"
  "uint8 STANDING=2\n"
  "uint8 WALKING=3\n"
  "uint8 ANIMATING=4\n"
  "\n"
  "uint8 state_id";

static char msg_encoding[] = "msg";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
hexapod_custom_msgs__msg__LocomotionState__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {hexapod_custom_msgs__msg__LocomotionState__TYPE_NAME, 39, 39},
    {msg_encoding, 3, 3},
    {toplevel_type_raw_source, 95, 95},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
hexapod_custom_msgs__msg__LocomotionState__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[1];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 1, 1};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *hexapod_custom_msgs__msg__LocomotionState__get_individual_type_description_source(NULL),
    constructed = true;
  }
  return &source_sequence;
}
