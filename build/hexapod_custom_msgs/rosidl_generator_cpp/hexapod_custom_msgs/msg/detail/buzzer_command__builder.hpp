// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from hexapod_custom_msgs:msg/BuzzerCommand.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "hexapod_custom_msgs/msg/buzzer_command.hpp"


#ifndef HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__BUILDER_HPP_
#define HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "hexapod_custom_msgs/msg/detail/buzzer_command__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace hexapod_custom_msgs
{

namespace msg
{

namespace builder
{

class Init_BuzzerCommand_command_id
{
public:
  Init_BuzzerCommand_command_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::hexapod_custom_msgs::msg::BuzzerCommand command_id(::hexapod_custom_msgs::msg::BuzzerCommand::_command_id_type arg)
  {
    msg_.command_id = std::move(arg);
    return std::move(msg_);
  }

private:
  ::hexapod_custom_msgs::msg::BuzzerCommand msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::hexapod_custom_msgs::msg::BuzzerCommand>()
{
  return hexapod_custom_msgs::msg::builder::Init_BuzzerCommand_command_id();
}

}  // namespace hexapod_custom_msgs

#endif  // HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__BUILDER_HPP_
