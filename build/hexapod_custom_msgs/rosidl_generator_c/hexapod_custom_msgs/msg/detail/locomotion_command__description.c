// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from hexapod_custom_msgs:msg/LocomotionCommand.idl
// generated code does not contain a copyright notice

#include "hexapod_custom_msgs/msg/detail/locomotion_command__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_hexapod_custom_msgs
const rosidl_type_hash_t *
hexapod_custom_msgs__msg__LocomotionCommand__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0x55, 0x6b, 0x33, 0x6c, 0xa8, 0xad, 0x89, 0xaf,
      0x04, 0x9b, 0x83, 0x84, 0x47, 0x4b, 0x6c, 0x2a,
      0xfc, 0x64, 0xb4, 0xba, 0x45, 0xd0, 0xda, 0xcd,
      0x1a, 0x76, 0xbc, 0x1f, 0x23, 0xb9, 0xdd, 0x54,
    }};
  return &hash;
}

#include <assert.h>
#include <string.h>

// Include directives for referenced types

// Hashes for external referenced types
#ifndef NDEBUG
#endif

static char hexapod_custom_msgs__msg__LocomotionCommand__TYPE_NAME[] = "hexapod_custom_msgs/msg/LocomotionCommand";

// Define type names, field names, and default values
static char hexapod_custom_msgs__msg__LocomotionCommand__FIELD_NAME__command_id[] = "command_id";

static rosidl_runtime_c__type_description__Field hexapod_custom_msgs__msg__LocomotionCommand__FIELDS[] = {
  {
    {hexapod_custom_msgs__msg__LocomotionCommand__FIELD_NAME__command_id, 10, 10},
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
hexapod_custom_msgs__msg__LocomotionCommand__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {hexapod_custom_msgs__msg__LocomotionCommand__TYPE_NAME, 41, 41},
      {hexapod_custom_msgs__msg__LocomotionCommand__FIELDS, 1, 1},
    },
    {NULL, 0, 0},
  };
  if (!constructed) {
    constructed = true;
  }
  return &description;
}

static char toplevel_type_raw_source[] =
  "uint8 CMD_STAND_UP=1\n"
  "uint8 CMD_SIT_DOWN=2\n"
  "uint8 CMD_CHANGE_GAIT=3\n"
  "uint8 CMD_PLAY_ANIMATION=4\n"
  "\n"
  "uint8 command_id";

static char msg_encoding[] = "msg";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
hexapod_custom_msgs__msg__LocomotionCommand__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {hexapod_custom_msgs__msg__LocomotionCommand__TYPE_NAME, 41, 41},
    {msg_encoding, 3, 3},
    {toplevel_type_raw_source, 110, 110},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
hexapod_custom_msgs__msg__LocomotionCommand__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[1];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 1, 1};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *hexapod_custom_msgs__msg__LocomotionCommand__get_individual_type_description_source(NULL),
    constructed = true;
  }
  return &source_sequence;
}
