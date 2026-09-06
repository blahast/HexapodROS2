# generated from rosidl_generator_py/resource/_idl.py.em
# with input from hexapod_custom_msgs:msg/LocomotionState.idl
# generated code does not contain a copyright notice

# This is being done at the module level and not on the instance level to avoid looking
# for the same variable multiple times on each instance. This variable is not supposed to
# change during runtime so it makes sense to only look for it once.
from os import getenv

ros_python_check_fields = getenv('ROS_PYTHON_CHECK_FIELDS', default='')


# Import statements for member types

import builtins  # noqa: E402, I100

import rosidl_parser.definition  # noqa: E402, I100


class Metaclass_LocomotionState(type):
    """Metaclass of message 'LocomotionState'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
        'INIT': 0,
        'SITTING': 1,
        'STANDING': 2,
        'WALKING': 3,
        'ANIMATING': 4,
    }

    @classmethod
    def __import_type_support__(cls):
        try:
            from rosidl_generator_py import import_type_support
            module = import_type_support('hexapod_custom_msgs')
        except ImportError:
            import logging
            import traceback
            logger = logging.getLogger(
                'hexapod_custom_msgs.msg.LocomotionState')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__msg__locomotion_state
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__msg__locomotion_state
            cls._CONVERT_TO_PY = module.convert_to_py_msg__msg__locomotion_state
            cls._TYPE_SUPPORT = module.type_support_msg__msg__locomotion_state
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__msg__locomotion_state

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
            'INIT': cls.__constants['INIT'],
            'SITTING': cls.__constants['SITTING'],
            'STANDING': cls.__constants['STANDING'],
            'WALKING': cls.__constants['WALKING'],
            'ANIMATING': cls.__constants['ANIMATING'],
        }

    @property
    def INIT(self):
        """Message constant 'INIT'."""
        return Metaclass_LocomotionState.__constants['INIT']

    @property
    def SITTING(self):
        """Message constant 'SITTING'."""
        return Metaclass_LocomotionState.__constants['SITTING']

    @property
    def STANDING(self):
        """Message constant 'STANDING'."""
        return Metaclass_LocomotionState.__constants['STANDING']

    @property
    def WALKING(self):
        """Message constant 'WALKING'."""
        return Metaclass_LocomotionState.__constants['WALKING']

    @property
    def ANIMATING(self):
        """Message constant 'ANIMATING'."""
        return Metaclass_LocomotionState.__constants['ANIMATING']


class LocomotionState(metaclass=Metaclass_LocomotionState):
    """
    Message class 'LocomotionState'.

    Constants:
      INIT
      SITTING
      STANDING
      WALKING
      ANIMATING
    """

    __slots__ = [
        '_state_id',
        '_check_fields',
    ]

    _fields_and_field_types = {
        'state_id': 'uint8',
    }

    # This attribute is used to store an rosidl_parser.definition variable
    # related to the data type of each of the components the message.
    SLOT_TYPES = (
        rosidl_parser.definition.BasicType('uint8'),  # noqa: E501
    )

    def __init__(self, **kwargs):
        if 'check_fields' in kwargs:
            self._check_fields = kwargs['check_fields']
        else:
            self._check_fields = ros_python_check_fields == '1'
        if self._check_fields:
            assert all('_' + key in self.__slots__ for key in kwargs.keys()), \
                'Invalid arguments passed to constructor: %s' % \
                ', '.join(sorted(k for k in kwargs.keys() if '_' + k not in self.__slots__))
        self.state_id = kwargs.get('state_id', int())

    def __repr__(self):
        typename = self.__class__.__module__.split('.')
        typename.pop()
        typename.append(self.__class__.__name__)
        args = []
        for s, t in zip(self.get_fields_and_field_types().keys(), self.SLOT_TYPES):
            field = getattr(self, s)
            fieldstr = repr(field)
            # We use Python array type for fields that can be directly stored
            # in them, and "normal" sequences for everything else.  If it is
            # a type that we store in an array, strip off the 'array' portion.
            if (
                isinstance(t, rosidl_parser.definition.AbstractSequence) and
                isinstance(t.value_type, rosidl_parser.definition.BasicType) and
                t.value_type.typename in ['float', 'double', 'int8', 'uint8', 'int16', 'uint16', 'int32', 'uint32', 'int64', 'uint64']
            ):
                if len(field) == 0:
                    fieldstr = '[]'
                else:
                    if self._check_fields:
                        assert fieldstr.startswith('array(')
                    prefix = "array('X', "
                    suffix = ')'
                    fieldstr = fieldstr[len(prefix):-len(suffix)]
            args.append(s + '=' + fieldstr)
        return '%s(%s)' % ('.'.join(typename), ', '.join(args))

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        if self.state_id != other.state_id:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def state_id(self):
        """Message field 'state_id'."""
        return self._state_id

    @state_id.setter
    def state_id(self, value):
        if self._check_fields:
            assert \
                isinstance(value, int), \
                "The 'state_id' field must be of type 'int'"
            assert value >= 0 and value < 256, \
                "The 'state_id' field must be an unsigned integer in [0, 255]"
        self._state_id = value
