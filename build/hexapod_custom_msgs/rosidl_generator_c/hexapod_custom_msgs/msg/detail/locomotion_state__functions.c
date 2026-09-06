// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from hexapod_custom_msgs:msg/LocomotionState.idl
// generated code does not contain a copyright notice
#include "hexapod_custom_msgs/msg/detail/locomotion_state__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


bool
hexapod_custom_msgs__msg__LocomotionState__init(hexapod_custom_msgs__msg__LocomotionState * msg)
{
  if (!msg) {
    return false;
  }
  // state_id
  return true;
}

void
hexapod_custom_msgs__msg__LocomotionState__fini(hexapod_custom_msgs__msg__LocomotionState * msg)
{
  if (!msg) {
    return;
  }
  // state_id
}

bool
hexapod_custom_msgs__msg__LocomotionState__are_equal(const hexapod_custom_msgs__msg__LocomotionState * lhs, const hexapod_custom_msgs__msg__LocomotionState * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // state_id
  if (lhs->state_id != rhs->state_id) {
    return false;
  }
  return true;
}

bool
hexapod_custom_msgs__msg__LocomotionState__copy(
  const hexapod_custom_msgs__msg__LocomotionState * input,
  hexapod_custom_msgs__msg__LocomotionState * output)
{
  if (!input || !output) {
    return false;
  }
  // state_id
  output->state_id = input->state_id;
  return true;
}

hexapod_custom_msgs__msg__LocomotionState *
hexapod_custom_msgs__msg__LocomotionState__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  hexapod_custom_msgs__msg__LocomotionState * msg = (hexapod_custom_msgs__msg__LocomotionState *)allocator.allocate(sizeof(hexapod_custom_msgs__msg__LocomotionState), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(hexapod_custom_msgs__msg__LocomotionState));
  bool success = hexapod_custom_msgs__msg__LocomotionState__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
hexapod_custom_msgs__msg__LocomotionState__destroy(hexapod_custom_msgs__msg__LocomotionState * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    hexapod_custom_msgs__msg__LocomotionState__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
hexapod_custom_msgs__msg__LocomotionState__Sequence__init(hexapod_custom_msgs__msg__LocomotionState__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  hexapod_custom_msgs__msg__LocomotionState * data = NULL;

  if (size) {
    if (size > SIZE_MAX / sizeof(hexapod_custom_msgs__msg__LocomotionState)) {
      return false;
    }
    data = (hexapod_custom_msgs__msg__LocomotionState *)allocator.zero_allocate(size, sizeof(hexapod_custom_msgs__msg__LocomotionState), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = hexapod_custom_msgs__msg__LocomotionState__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        hexapod_custom_msgs__msg__LocomotionState__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
hexapod_custom_msgs__msg__LocomotionState__Sequence__fini(hexapod_custom_msgs__msg__LocomotionState__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      hexapod_custom_msgs__msg__LocomotionState__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

hexapod_custom_msgs__msg__LocomotionState__Sequence *
hexapod_custom_msgs__msg__LocomotionState__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  hexapod_custom_msgs__msg__LocomotionState__Sequence * array = (hexapod_custom_msgs__msg__LocomotionState__Sequence *)allocator.allocate(sizeof(hexapod_custom_msgs__msg__LocomotionState__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = hexapod_custom_msgs__msg__LocomotionState__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
hexapod_custom_msgs__msg__LocomotionState__Sequence__destroy(hexapod_custom_msgs__msg__LocomotionState__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    hexapod_custom_msgs__msg__LocomotionState__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
hexapod_custom_msgs__msg__LocomotionState__Sequence__are_equal(const hexapod_custom_msgs__msg__LocomotionState__Sequence * lhs, const hexapod_custom_msgs__msg__LocomotionState__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!hexapod_custom_msgs__msg__LocomotionState__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
hexapod_custom_msgs__msg__LocomotionState__Sequence__copy(
  const hexapod_custom_msgs__msg__LocomotionState__Sequence * input,
  hexapod_custom_msgs__msg__LocomotionState__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    if (input->size > SIZE_MAX / sizeof(hexapod_custom_msgs__msg__LocomotionState)) {
      return false;
    }
    const size_t allocation_size =
      input->size * sizeof(hexapod_custom_msgs__msg__LocomotionState);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    hexapod_custom_msgs__msg__LocomotionState * data =
      (hexapod_custom_msgs__msg__LocomotionState *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!hexapod_custom_msgs__msg__LocomotionState__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          hexapod_custom_msgs__msg__LocomotionState__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!hexapod_custom_msgs__msg__LocomotionState__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
