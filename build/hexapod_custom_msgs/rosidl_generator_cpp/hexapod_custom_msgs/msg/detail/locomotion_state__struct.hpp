// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from hexapod_custom_msgs:msg/LocomotionState.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "hexapod_custom_msgs/msg/locomotion_state.hpp"


#ifndef HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__STRUCT_HPP_
#define HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__hexapod_custom_msgs__msg__LocomotionState __attribute__((deprecated))
#else
# define DEPRECATED__hexapod_custom_msgs__msg__LocomotionState __declspec(deprecated)
#endif

namespace hexapod_custom_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct LocomotionState_
{
  using Type = LocomotionState_<ContainerAllocator>;

  explicit LocomotionState_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->state_id = 0;
    }
  }

  explicit LocomotionState_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->state_id = 0;
    }
  }

  // field types and members
  using _state_id_type =
    uint8_t;
  _state_id_type state_id;

  // setters for named parameter idiom
  Type & set__state_id(
    const uint8_t & _arg)
  {
    this->state_id = _arg;
    return *this;
  }

  // constant declarations
  static constexpr uint8_t INIT =
    0u;
  static constexpr uint8_t SITTING =
    1u;
  static constexpr uint8_t STANDING =
    2u;
  static constexpr uint8_t WALKING =
    3u;
  static constexpr uint8_t ANIMATING =
    4u;

  // pointer types
  using RawPtr =
    hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator> *;
  using ConstRawPtr =
    const hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__hexapod_custom_msgs__msg__LocomotionState
    std::shared_ptr<hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__hexapod_custom_msgs__msg__LocomotionState
    std::shared_ptr<hexapod_custom_msgs::msg::LocomotionState_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const LocomotionState_ & other) const
  {
    if (this->state_id != other.state_id) {
      return false;
    }
    return true;
  }
  bool operator!=(const LocomotionState_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct LocomotionState_

// alias to use template instance with default allocator
using LocomotionState =
  hexapod_custom_msgs::msg::LocomotionState_<std::allocator<void>>;

// constant definitions
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t LocomotionState_<ContainerAllocator>::INIT;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t LocomotionState_<ContainerAllocator>::SITTING;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t LocomotionState_<ContainerAllocator>::STANDING;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t LocomotionState_<ContainerAllocator>::WALKING;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t LocomotionState_<ContainerAllocator>::ANIMATING;
#endif  // __cplusplus < 201703L

}  // namespace msg

}  // namespace hexapod_custom_msgs

#endif  // HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__LOCOMOTION_STATE__STRUCT_HPP_
