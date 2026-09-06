# generated from rosidl_generator_py/resource/_idl.py.em
# with input from hexapod_custom_msgs:msg/BuzzerCommand.idl
# generated code does not contain a copyright notice

# This is being done at the module level and not on the instance level to avoid looking
# for the same variable multiple times on each instance. This variable is not supposed to
# change during runtime so it makes sense to only look for it once.
from os import getenv

ros_python_check_fields = getenv('ROS_PYTHON_CHECK_FIELDS', default='')


# Import statements for member types

import builtins  # noqa: E402, I100

import rosidl_parser.definition  # noqa: E402, I100


class Metaclass_BuzzerCommand(type):
    """Metaclass of message 'BuzzerCommand'."""

    _CREATE_ROS_MESSAGE = None
    _CONVERT_FROM_PY = None
    _CONVERT_TO_PY = None
    _DESTROY_ROS_MESSAGE = None
    _TYPE_SUPPORT = None

    __constants = {
        'BEEP': 1,
        'STARTUP': 2,
        'SHUTDOWN': 3,
        'ERROR': 4,
        'MELODY': 5,
        'BT_CONNECTED': 6,
        'BT_DISCONNECTED': 7,
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
                'hexapod_custom_msgs.msg.BuzzerCommand')
            logger.debug(
                'Failed to import needed modules for type support:\n' +
                traceback.format_exc())
        else:
            cls._CREATE_ROS_MESSAGE = module.create_ros_message_msg__msg__buzzer_command
            cls._CONVERT_FROM_PY = module.convert_from_py_msg__msg__buzzer_command
            cls._CONVERT_TO_PY = module.convert_to_py_msg__msg__buzzer_command
            cls._TYPE_SUPPORT = module.type_support_msg__msg__buzzer_command
            cls._DESTROY_ROS_MESSAGE = module.destroy_ros_message_msg__msg__buzzer_command

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        # list constant names here so that they appear in the help text of
        # the message class under "Data and other attributes defined here:"
        # as well as populate each message instance
        return {
            'BEEP': cls.__constants['BEEP'],
            'STARTUP': cls.__constants['STARTUP'],
            'SHUTDOWN': cls.__constants['SHUTDOWN'],
            'ERROR': cls.__constants['ERROR'],
            'MELODY': cls.__constants['MELODY'],
            'BT_CONNECTED': cls.__constants['BT_CONNECTED'],
            'BT_DISCONNECTED': cls.__constants['BT_DISCONNECTED'],
        }

    @property
    def BEEP(self):
        """Message constant 'BEEP'."""
        return Metaclass_BuzzerCommand.__constants['BEEP']

    @property
    def STARTUP(self):
        """Message constant 'STARTUP'."""
        return Metaclass_BuzzerCommand.__constants['STARTUP']

    @property
    def SHUTDOWN(self):
        """Message constant 'SHUTDOWN'."""
        return Metaclass_BuzzerCommand.__constants['SHUTDOWN']

    @property
    def ERROR(self):
        """Message constant 'ERROR'."""
        return Metaclass_BuzzerCommand.__constants['ERROR']

    @property
    def MELODY(self):
        """Message constant 'MELODY'."""
        return Metaclass_BuzzerCommand.__constants['MELODY']

    @property
    def BT_CONNECTED(self):
        """Message constant 'BT_CONNECTED'."""
        return Metaclass_BuzzerCommand.__constants['BT_CONNECTED']

    @property
    def BT_DISCONNECTED(self):
        """Message constant 'BT_DISCONNECTED'."""
        return Metaclass_BuzzerCommand.__constants['BT_DISCONNECTED']


class BuzzerCommand(metaclass=Metaclass_BuzzerCommand):
    """
    Message class 'BuzzerCommand'.

    Constants:
      BEEP
      STARTUP
      SHUTDOWN
      ERROR
      MELODY
      BT_CONNECTED
      BT_DISCONNECTED
    """

    __slots__ = [
        '_command_id',
        '_check_fields',
    ]

    _fields_and_field_types = {
        'command_id': 'uint8',
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
        self.command_id = kwargs.get('command_id', int())

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
        if self.command_id != other.command_id:
            return False
        return True

    @classmethod
    def get_fields_and_field_types(cls):
        from copy import copy
        return copy(cls._fields_and_field_types)

    @builtins.property
    def command_id(self):
        """Message field 'command_id'."""
        return self._command_id

    @command_id.setter
    def command_id(self, value):
        if self._check_fields:
            assert \
                isinstance(value, int), \
                "The 'command_id' field must be of type 'int'"
            assert value >= 0 and value < 256, \
                "The 'command_id' field must be an unsigned integer in [0, 255]"
        self._command_id = value
