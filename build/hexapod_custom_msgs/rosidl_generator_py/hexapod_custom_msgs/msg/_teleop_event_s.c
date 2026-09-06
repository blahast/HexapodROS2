// generated from rosidl_generator_py/resource/_idl_support.c.em
// with input from hexapod_custom_msgs:msg/TeleopEvent.idl
// generated code does not contain a copyright notice
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <stdbool.h>
#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include "numpy/ndarrayobject.h"
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif
#include "rosidl_runtime_c/visibility_control.h"
#include "hexapod_custom_msgs/msg/detail/teleop_event__struct.h"
#include "hexapod_custom_msgs/msg/detail/teleop_event__functions.h"


ROSIDL_GENERATOR_C_EXPORT
bool hexapod_custom_msgs__msg__teleop_event__convert_from_py(PyObject * _pymsg, void * _ros_message)
{
  // check that the passed message is of the expected Python class
  {
    char full_classname_dest[50];
    {
      char * class_name = NULL;
      char * module_name = NULL;
      {
        PyObject * class_attr = PyObject_GetAttrString(_pymsg, "__class__");
        if (class_attr) {
          PyObject * name_attr = PyObject_GetAttrString(class_attr, "__name__");
          if (name_attr) {
            class_name = (char *)PyUnicode_1BYTE_DATA(name_attr);
            Py_DECREF(name_attr);
          }
          PyObject * module_attr = PyObject_GetAttrString(class_attr, "__module__");
          if (module_attr) {
            module_name = (char *)PyUnicode_1BYTE_DATA(module_attr);
            Py_DECREF(module_attr);
          }
          Py_DECREF(class_attr);
        }
      }
      if (!class_name || !module_name) {
        return false;
      }
      snprintf(full_classname_dest, sizeof(full_classname_dest), "%s.%s", module_name, class_name);
    }
    assert(strncmp("hexapod_custom_msgs.msg._teleop_event.TeleopEvent", full_classname_dest, 49) == 0);
  }
  hexapod_custom_msgs__msg__TeleopEvent * ros_message = _ros_message;
  {  // event_id
    PyObject * field = PyObject_GetAttrString(_pymsg, "event_id");
    if (!field) {
      return false;
    }
    assert(PyLong_Check(field));
    ros_message->event_id = (uint8_t)PyLong_AsUnsignedLong(field);
    Py_DECREF(field);
  }

  return true;
}

ROSIDL_GENERATOR_C_EXPORT
PyObject * hexapod_custom_msgs__msg__teleop_event__convert_to_py(void * raw_ros_message)
{
  /* NOTE(esteve): Call constructor of TeleopEvent */
  PyObject * _pymessage = NULL;
  {
    PyObject * pymessage_module = PyImport_ImportModule("hexapod_custom_msgs.msg._teleop_event");
    assert(pymessage_module);
    PyObject * pymessage_class = PyObject_GetAttrString(pymessage_module, "TeleopEvent");
    assert(pymessage_class);
    Py_DECREF(pymessage_module);
    _pymessage = PyObject_CallObject(pymessage_class, NULL);
    Py_DECREF(pymessage_class);
    if (!_pymessage) {
      return NULL;
    }
  }
  hexapod_custom_msgs__msg__TeleopEvent * ros_message = (hexapod_custom_msgs__msg__TeleopEvent *)raw_ros_message;
  {  // event_id
    PyObject * field = NULL;
    field = PyLong_FromUnsignedLong(ros_message->event_id);
    {
      int rc = PyObject_SetAttrString(_pymessage, "event_id", field);
      Py_DECREF(field);
      if (rc) {
        return NULL;
      }
    }
  }

  // ownership of _pymessage is transferred to the caller
  return _pymessage;
}
