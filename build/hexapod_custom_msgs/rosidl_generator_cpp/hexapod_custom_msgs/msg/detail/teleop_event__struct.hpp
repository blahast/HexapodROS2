// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from hexapod_custom_msgs:msg/TeleopEvent.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "hexapod_custom_msgs/msg/teleop_event.hpp"


#ifndef HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__TELEOP_EVENT__STRUCT_HPP_
#define HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__TELEOP_EVENT__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__hexapod_custom_msgs__msg__TeleopEvent __attribute__((deprecated))
#else
# define DEPRECATED__hexapod_custom_msgs__msg__TeleopEvent __declspec(deprecated)
#endif

namespace hexapod_custom_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct TeleopEvent_
{
  using Type = TeleopEvent_<ContainerAllocator>;

  explicit TeleopEvent_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->event_id = 0;
    }
  }

  explicit TeleopEvent_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->event_id = 0;
    }
  }

  // field types and members
  using _event_id_type =
    uint8_t;
  _event_id_type event_id;

  // setters for named parameter idiom
  Type & set__event_id(
    const uint8_t & _arg)
  {
    this->event_id = _arg;
    return *this;
  }

  // constant declarations
  static constexpr uint8_t BT_CONNECTED =
    1u;
  static constexpr uint8_t BT_DISCONNECTED =
    2u;
  static constexpr uint8_t TOGGLE_STAND =
    3u;
  static constexpr uint8_t CHANGE_GAIT =
    4u;
  static constexpr uint8_t PLAY_ANIMATION =
    5u;

  // pointer types
  using RawPtr =
    hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator> *;
  using ConstRawPtr =
    const hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__hexapod_custom_msgs__msg__TeleopEvent
    std::shared_ptr<hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__hexapod_custom_msgs__msg__TeleopEvent
    std::shared_ptr<hexapod_custom_msgs::msg::TeleopEvent_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const TeleopEvent_ & other) const
  {
    if (this->event_id != other.event_id) {
      return false;
    }
    return true;
  }
  bool operator!=(const TeleopEvent_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct TeleopEvent_

// alias to use template instance with default allocator
using TeleopEvent =
  hexapod_custom_msgs::msg::TeleopEvent_<std::allocator<void>>;

// constant definitions
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t TeleopEvent_<ContainerAllocator>::BT_CONNECTED;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t TeleopEvent_<ContainerAllocator>::BT_DISCONNECTED;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t TeleopEvent_<ContainerAllocator>::TOGGLE_STAND;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t TeleopEvent_<ContainerAllocator>::CHANGE_GAIT;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t TeleopEvent_<ContainerAllocator>::PLAY_ANIMATION;
#endif  // __cplusplus < 201703L

}  // namespace msg

}  // namespace hexapod_custom_msgs

#endif  // HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__TELEOP_EVENT__STRUCT_HPP_
