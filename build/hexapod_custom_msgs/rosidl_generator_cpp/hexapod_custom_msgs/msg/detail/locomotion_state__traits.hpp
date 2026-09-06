// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from hexapod_custom_msgs:msg/LocomotionState.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "hexapod_custom_msgs/msg/locomotion_state.hpp"


#ifndef HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__TRAITS_HPP_
#define HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "hexapod_custom_msgs/msg/detail/locomotion_state__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace hexapod_custom_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const LocomotionState & msg,
  std::ostream & out)
{
  out << "{";
  // member: state_id
  {
    out << "state_id: ";
    rosidl_generator_traits::value_to_yaml(msg.state_id, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const LocomotionState & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: state_id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "state_id: ";
    rosidl_generator_traits::value_to_yaml(msg.state_id, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const LocomotionState & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace hexapod_custom_msgs

namespace rosidl_generator_traits
{

[[deprecated("use hexapod_custom_msgs::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const hexapod_custom_msgs::msg::LocomotionState & msg,
  std::ostream & out, size_t indentation = 0)
{
  hexapod_custom_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use hexapod_custom_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const hexapod_custom_msgs::msg::LocomotionState & msg)
{
  return hexapod_custom_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<hexapod_custom_msgs::msg::LocomotionState>()
{
  return "hexapod_custom_msgs::msg::LocomotionState";
}

template<>
inline const char * name<hexapod_custom_msgs::msg::LocomotionState>()
{
  return "hexapod_custom_msgs/msg/LocomotionState";
}

template<>
struct has_fixed_size<hexapod_custom_msgs::msg::LocomotionState>
  : std::integral_constant<bool, true> {};

template<>
struct has_bounded_size<hexapod_custom_msgs::msg::LocomotionState>
  : std::integral_constant<bool, true> {};

template<>
struct is_message<hexapod_custom_msgs::msg::LocomotionState>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__TRAITS_HPP_
