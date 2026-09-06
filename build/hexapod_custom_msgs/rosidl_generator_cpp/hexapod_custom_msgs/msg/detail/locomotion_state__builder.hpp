// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from hexapod_custom_msgs:msg/LocomotionState.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "hexapod_custom_msgs/msg/locomotion_state.hpp"


#ifndef HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__BUILDER_HPP_
#define HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "hexapod_custom_msgs/msg/detail/locomotion_state__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace hexapod_custom_msgs
{

namespace msg
{

namespace builder
{

class Init_LocomotionState_state_id
{
public:
  Init_LocomotionState_state_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::hexapod_custom_msgs::msg::LocomotionState state_id(::hexapod_custom_msgs::msg::LocomotionState::_state_id_type arg)
  {
    msg_.state_id = std::move(arg);
    return std::move(msg_);
  }

private:
  ::hexapod_custom_msgs::msg::LocomotionState msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::hexapod_custom_msgs::msg::LocomotionState>()
{
  return hexapod_custom_msgs::msg::builder::Init_LocomotionState_state_id();
}

}  // namespace hexapod_custom_msgs

#endif  // HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__BUILDER_HPP_
