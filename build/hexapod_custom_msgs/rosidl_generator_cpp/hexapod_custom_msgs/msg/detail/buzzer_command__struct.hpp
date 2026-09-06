// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from hexapod_custom_msgs:msg/BuzzerCommand.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "hexapod_custom_msgs/msg/buzzer_command.hpp"


#ifndef HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__STRUCT_HPP_
#define HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__hexapod_custom_msgs__msg__BuzzerCommand __attribute__((deprecated))
#else
# define DEPRECATED__hexapod_custom_msgs__msg__BuzzerCommand __declspec(deprecated)
#endif

namespace hexapod_custom_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct BuzzerCommand_
{
  using Type = BuzzerCommand_<ContainerAllocator>;

  explicit BuzzerCommand_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->command_id = 0;
    }
  }

  explicit BuzzerCommand_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->command_id = 0;
    }
  }

  // field types and members
  using _command_id_type =
    uint8_t;
  _command_id_type command_id;

  // setters for named parameter idiom
  Type & set__command_id(
    const uint8_t & _arg)
  {
    this->command_id = _arg;
    return *this;
  }

  // constant declarations
  static constexpr uint8_t BEEP =
    1u;
  static constexpr uint8_t STARTUP =
    2u;
  static constexpr uint8_t SHUTDOWN =
    3u;
  // guard against 'ERROR' being predefined by MSVC by temporarily undefining it
#if defined(_WIN32)
#  if defined(ERROR)
#    pragma push_macro("ERROR")
#    undef ERROR
#  endif
#endif
  static constexpr uint8_t ERROR =
    4u;
#if defined(_WIN32)
#  pragma warning(suppress : 4602)
#  pragma pop_macro("ERROR")
#endif
  static constexpr uint8_t MELODY =
    5u;
  static constexpr uint8_t BT_CONNECTED =
    6u;
  static constexpr uint8_t BT_DISCONNECTED =
    7u;

  // pointer types
  using RawPtr =
    hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator> *;
  using ConstRawPtr =
    const hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__hexapod_custom_msgs__msg__BuzzerCommand
    std::shared_ptr<hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__hexapod_custom_msgs__msg__BuzzerCommand
    std::shared_ptr<hexapod_custom_msgs::msg::BuzzerCommand_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const BuzzerCommand_ & other) const
  {
    if (this->command_id != other.command_id) {
      return false;
    }
    return true;
  }
  bool operator!=(const BuzzerCommand_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct BuzzerCommand_

// alias to use template instance with default allocator
using BuzzerCommand =
  hexapod_custom_msgs::msg::BuzzerCommand_<std::allocator<void>>;

// constant definitions
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t BuzzerCommand_<ContainerAllocator>::BEEP;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t BuzzerCommand_<ContainerAllocator>::STARTUP;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t BuzzerCommand_<ContainerAllocator>::SHUTDOWN;
#endif  // __cplusplus < 201703L
// guard against 'ERROR' being predefined by MSVC by temporarily undefining it
#if defined(_WIN32)
#  if defined(ERROR)
#    pragma push_macro("ERROR")
#    undef ERROR
#  endif
#endif
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t BuzzerCommand_<ContainerAllocator>::ERROR;
#endif  // __cplusplus < 201703L
#if defined(_WIN32)
#  pragma warning(suppress : 4602)
#  pragma pop_macro("ERROR")
#endif
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t BuzzerCommand_<ContainerAllocator>::MELODY;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t BuzzerCommand_<ContainerAllocator>::BT_CONNECTED;
#endif  // __cplusplus < 201703L
#if __cplusplus < 201703L
// static constexpr member variable definitions are only needed in C++14 and below, deprecated in C++17
template<typename ContainerAllocator>
constexpr uint8_t BuzzerCommand_<ContainerAllocator>::BT_DISCONNECTED;
#endif  // __cplusplus < 201703L

}  // namespace msg

}  // namespace hexapod_custom_msgs

#endif  // HEXAPOD_CUSTOM_MSGS__MSG__DETAIL__BUZZER_COMMAND__STRUCT_HPP_
