// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from hexapod_custom_msgs:msg/TeleopEvent.idl
// generated code does not contain a copyright notice

#include "hexapod_custom_msgs/msg/detail/teleop_event__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_hexapod_custom_msgs
const rosidl_type_hash_t *
hexapod_custom_msgs__msg__TeleopEvent__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0x07, 0xd3, 0xee, 0x2c, 0x0a, 0x44, 0xdf, 0x28,
      0xa1, 0x6e, 0xcd, 0x20, 0x3b, 0x03, 0xeb, 0xa1,
      0x9e, 0xf5, 0x2c, 0x1d, 0x1f, 0xf6, 0x86, 0x6b,
      0xa4, 0x9b, 0x76, 0x60, 0xcf, 0x5b, 0xf6, 0x2c,
    }};
  return &hash;
}

#include <assert.h>
#include <string.h>

// Include directives for referenced types

// Hashes for external referenced types
#ifndef NDEBUG
#endif

static char hexapod_custom_msgs__msg__TeleopEvent__TYPE_NAME[] = "hexapod_custom_msgs/msg/TeleopEvent";

// Define type names, field names, and default values
static char hexapod_custom_msgs__msg__TeleopEvent__FIELD_NAME__event_id[] = "event_id";

static rosidl_runtime_c__type_description__Field hexapod_custom_msgs__msg__TeleopEvent__FIELDS[] = {
  {
    {hexapod_custom_msgs__msg__TeleopEvent__FIELD_NAME__event_id, 8, 8},
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
hexapod_custom_msgs__msg__TeleopEvent__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {hexapod_custom_msgs__msg__TeleopEvent__TYPE_NAME, 35, 35},
      {hexapod_custom_msgs__msg__TeleopEvent__FIELDS, 1, 1},
    },
    {NULL, 0, 0},
  };
  if (!constructed) {
    constructed = true;
  }
  return &description;
}

static char toplevel_type_raw_source[] =
  "uint8 BT_CONNECTED=1\n"
  "uint8 BT_DISCONNECTED=2\n"
  "uint8 TOGGLE_STAND=3\n"
  "uint8 CHANGE_GAIT=4\n"
  "uint8 PLAY_ANIMATION=5\n"
  "\n"
  "uint8 event_id";

static char msg_encoding[] = "msg";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
hexapod_custom_msgs__msg__TeleopEvent__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {hexapod_custom_msgs__msg__TeleopEvent__TYPE_NAME, 35, 35},
    {msg_encoding, 3, 3},
    {toplevel_type_raw_source, 124, 124},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
hexapod_custom_msgs__msg__TeleopEvent__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[1];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 1, 1};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *hexapod_custom_msgs__msg__TeleopEvent__get_individual_type_description_source(NULL),
    constructed = true;
  }
  return &source_sequence;
}
