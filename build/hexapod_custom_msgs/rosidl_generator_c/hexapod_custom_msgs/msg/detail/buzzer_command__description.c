// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from hexapod_custom_msgs:msg/BuzzerCommand.idl
// generated code does not contain a copyright notice

#include "hexapod_custom_msgs/msg/detail/buzzer_command__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_hexapod_custom_msgs
const rosidl_type_hash_t *
hexapod_custom_msgs__msg__BuzzerCommand__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0x38, 0x13, 0x07, 0x17, 0xb8, 0x5e, 0x4d, 0x0e,
      0x2b, 0x1f, 0x56, 0x91, 0x27, 0x7a, 0x1a, 0x9a,
      0x39, 0xe4, 0xc4, 0xdc, 0x4c, 0xf2, 0x30, 0x0b,
      0x5d, 0xee, 0x27, 0xd3, 0xcc, 0xbc, 0xc7, 0xd9,
    }};
  return &hash;
}

#include <assert.h>
#include <string.h>

// Include directives for referenced types

// Hashes for external referenced types
#ifndef NDEBUG
#endif

static char hexapod_custom_msgs__msg__BuzzerCommand__TYPE_NAME[] = "hexapod_custom_msgs/msg/BuzzerCommand";

// Define type names, field names, and default values
static char hexapod_custom_msgs__msg__BuzzerCommand__FIELD_NAME__command_id[] = "command_id";

static rosidl_runtime_c__type_description__Field hexapod_custom_msgs__msg__BuzzerCommand__FIELDS[] = {
  {
    {hexapod_custom_msgs__msg__BuzzerCommand__FIELD_NAME__command_id, 10, 10},
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
hexapod_custom_msgs__msg__BuzzerCommand__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {hexapod_custom_msgs__msg__BuzzerCommand__TYPE_NAME, 37, 37},
      {hexapod_custom_msgs__msg__BuzzerCommand__FIELDS, 1, 1},
    },
    {NULL, 0, 0},
  };
  if (!constructed) {
    constructed = true;
  }
  return &description;
}

static char toplevel_type_raw_source[] =
  "uint8 BEEP=1\n"
  "uint8 STARTUP=2\n"
  "uint8 SHUTDOWN=3\n"
  "uint8 ERROR=4\n"
  "uint8 MELODY=5\n"
  "uint8 BT_CONNECTED=6\n"
  "uint8 BT_DISCONNECTED=7\n"
  "\n"
  "uint8 command_id";

static char msg_encoding[] = "msg";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
hexapod_custom_msgs__msg__BuzzerCommand__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {hexapod_custom_msgs__msg__BuzzerCommand__TYPE_NAME, 37, 37},
    {msg_encoding, 3, 3},
    {toplevel_type_raw_source, 137, 137},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
hexapod_custom_msgs__msg__BuzzerCommand__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[1];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 1, 1};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *hexapod_custom_msgs__msg__BuzzerCommand__get_individual_type_description_source(NULL),
    constructed = true;
  }
  return &source_sequence;
}
