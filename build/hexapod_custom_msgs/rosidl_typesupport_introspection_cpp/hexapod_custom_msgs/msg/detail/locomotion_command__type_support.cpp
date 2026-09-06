// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from hexapod_custom_msgs:msg/LocomotionCommand.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "hexapod_custom_msgs/msg/detail/locomotion_command__functions.h"
#include "hexapod_custom_msgs/msg/detail/locomotion_command__struct.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

namespace hexapod_custom_msgs
{

namespace msg
{

namespace rosidl_typesupport_introspection_cpp
{

void LocomotionCommand_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) hexapod_custom_msgs::msg::LocomotionCommand(_init);
}

void LocomotionCommand_fini_function(void * message_memory)
{
  auto typed_message = static_cast<hexapod_custom_msgs::msg::LocomotionCommand *>(message_memory);
  typed_message->~LocomotionCommand();
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember LocomotionCommand_message_member_array[1] = {
  {
    "command_id",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(hexapod_custom_msgs::msg::LocomotionCommand, command_id),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers LocomotionCommand_message_members = {
  "hexapod_custom_msgs::msg",  // message namespace
  "LocomotionCommand",  // message name
  1,  // number of fields
  sizeof(hexapod_custom_msgs::msg::LocomotionCommand),
  false,  // has_any_key_member_
  LocomotionCommand_message_member_array,  // message members
  LocomotionCommand_init_function,  // function to initialize message memory (memory has to be allocated)
  LocomotionCommand_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t LocomotionCommand_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &LocomotionCommand_message_members,
  get_message_typesupport_handle_function,
  &hexapod_custom_msgs__msg__LocomotionCommand__get_type_hash,
  &hexapod_custom_msgs__msg__LocomotionCommand__get_type_description,
  &hexapod_custom_msgs__msg__LocomotionCommand__get_type_description_sources,
};

}  // namespace rosidl_typesupport_introspection_cpp

}  // namespace msg

}  // namespace hexapod_custom_msgs


namespace rosidl_typesupport_introspection_cpp
{

template<>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<hexapod_custom_msgs::msg::LocomotionCommand>()
{
  return &::hexapod_custom_msgs::msg::rosidl_typesupport_introspection_cpp::LocomotionCommand_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, hexapod_custom_msgs, msg, LocomotionCommand)() {
  return &::hexapod_custom_msgs::msg::rosidl_typesupport_introspection_cpp::LocomotionCommand_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
