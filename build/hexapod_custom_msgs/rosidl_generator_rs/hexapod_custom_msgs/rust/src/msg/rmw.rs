#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};


#[link(name = "hexapod_custom_msgs__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__hexapod_custom_msgs__msg__TeleopEvent() -> *const std::ffi::c_void;
}

#[link(name = "hexapod_custom_msgs__rosidl_generator_c")]
extern "C" {
    fn hexapod_custom_msgs__msg__TeleopEvent__init(msg: *mut TeleopEvent) -> bool;
    fn hexapod_custom_msgs__msg__TeleopEvent__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<TeleopEvent>, size: usize) -> bool;
    fn hexapod_custom_msgs__msg__TeleopEvent__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<TeleopEvent>);
    fn hexapod_custom_msgs__msg__TeleopEvent__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<TeleopEvent>, out_seq: *mut rosidl_runtime_rs::Sequence<TeleopEvent>) -> bool;
}

// Corresponds to hexapod_custom_msgs__msg__TeleopEvent
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct TeleopEvent {

    // This member is not documented.
    #[allow(missing_docs)]
    pub event_id: u8,

}

impl TeleopEvent {

    // This constant is not documented.
    #[allow(missing_docs)]
    pub const BT_CONNECTED: u8 = 1;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const BT_DISCONNECTED: u8 = 2;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const TOGGLE_STAND: u8 = 3;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const CHANGE_GAIT: u8 = 4;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const PLAY_ANIMATION: u8 = 5;

}


impl Default for TeleopEvent {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !hexapod_custom_msgs__msg__TeleopEvent__init(&mut msg as *mut _) {
        panic!("Call to hexapod_custom_msgs__msg__TeleopEvent__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for TeleopEvent {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__TeleopEvent__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__TeleopEvent__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__TeleopEvent__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for TeleopEvent {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for TeleopEvent where Self: Sized {
  const TYPE_NAME: &'static str = "hexapod_custom_msgs/msg/TeleopEvent";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__hexapod_custom_msgs__msg__TeleopEvent() }
  }
}


#[link(name = "hexapod_custom_msgs__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__hexapod_custom_msgs__msg__BuzzerCommand() -> *const std::ffi::c_void;
}

#[link(name = "hexapod_custom_msgs__rosidl_generator_c")]
extern "C" {
    fn hexapod_custom_msgs__msg__BuzzerCommand__init(msg: *mut BuzzerCommand) -> bool;
    fn hexapod_custom_msgs__msg__BuzzerCommand__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<BuzzerCommand>, size: usize) -> bool;
    fn hexapod_custom_msgs__msg__BuzzerCommand__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<BuzzerCommand>);
    fn hexapod_custom_msgs__msg__BuzzerCommand__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<BuzzerCommand>, out_seq: *mut rosidl_runtime_rs::Sequence<BuzzerCommand>) -> bool;
}

// Corresponds to hexapod_custom_msgs__msg__BuzzerCommand
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct BuzzerCommand {

    // This member is not documented.
    #[allow(missing_docs)]
    pub command_id: u8,

}

impl BuzzerCommand {

    // This constant is not documented.
    #[allow(missing_docs)]
    pub const BEEP: u8 = 1;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const STARTUP: u8 = 2;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const SHUTDOWN: u8 = 3;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const ERROR: u8 = 4;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const MELODY: u8 = 5;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const BT_CONNECTED: u8 = 6;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const BT_DISCONNECTED: u8 = 7;

}


impl Default for BuzzerCommand {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !hexapod_custom_msgs__msg__BuzzerCommand__init(&mut msg as *mut _) {
        panic!("Call to hexapod_custom_msgs__msg__BuzzerCommand__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for BuzzerCommand {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__BuzzerCommand__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__BuzzerCommand__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__BuzzerCommand__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for BuzzerCommand {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for BuzzerCommand where Self: Sized {
  const TYPE_NAME: &'static str = "hexapod_custom_msgs/msg/BuzzerCommand";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__hexapod_custom_msgs__msg__BuzzerCommand() }
  }
}


#[link(name = "hexapod_custom_msgs__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__hexapod_custom_msgs__msg__LocomotionState() -> *const std::ffi::c_void;
}

#[link(name = "hexapod_custom_msgs__rosidl_generator_c")]
extern "C" {
    fn hexapod_custom_msgs__msg__LocomotionState__init(msg: *mut LocomotionState) -> bool;
    fn hexapod_custom_msgs__msg__LocomotionState__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<LocomotionState>, size: usize) -> bool;
    fn hexapod_custom_msgs__msg__LocomotionState__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<LocomotionState>);
    fn hexapod_custom_msgs__msg__LocomotionState__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<LocomotionState>, out_seq: *mut rosidl_runtime_rs::Sequence<LocomotionState>) -> bool;
}

// Corresponds to hexapod_custom_msgs__msg__LocomotionState
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct LocomotionState {

    // This member is not documented.
    #[allow(missing_docs)]
    pub state_id: u8,

}

impl LocomotionState {

    // This constant is not documented.
    #[allow(missing_docs)]
    pub const INIT: u8 = 0;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const SITTING: u8 = 1;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const STANDING: u8 = 2;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const WALKING: u8 = 3;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const ANIMATING: u8 = 4;

}


impl Default for LocomotionState {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !hexapod_custom_msgs__msg__LocomotionState__init(&mut msg as *mut _) {
        panic!("Call to hexapod_custom_msgs__msg__LocomotionState__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for LocomotionState {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__LocomotionState__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__LocomotionState__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__LocomotionState__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for LocomotionState {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for LocomotionState where Self: Sized {
  const TYPE_NAME: &'static str = "hexapod_custom_msgs/msg/LocomotionState";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__hexapod_custom_msgs__msg__LocomotionState() }
  }
}


#[link(name = "hexapod_custom_msgs__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__hexapod_custom_msgs__msg__LocomotionCommand() -> *const std::ffi::c_void;
}

#[link(name = "hexapod_custom_msgs__rosidl_generator_c")]
extern "C" {
    fn hexapod_custom_msgs__msg__LocomotionCommand__init(msg: *mut LocomotionCommand) -> bool;
    fn hexapod_custom_msgs__msg__LocomotionCommand__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<LocomotionCommand>, size: usize) -> bool;
    fn hexapod_custom_msgs__msg__LocomotionCommand__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<LocomotionCommand>);
    fn hexapod_custom_msgs__msg__LocomotionCommand__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<LocomotionCommand>, out_seq: *mut rosidl_runtime_rs::Sequence<LocomotionCommand>) -> bool;
}

// Corresponds to hexapod_custom_msgs__msg__LocomotionCommand
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]


// This struct is not documented.
#[allow(missing_docs)]

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct LocomotionCommand {

    // This member is not documented.
    #[allow(missing_docs)]
    pub command_id: u8,

}

impl LocomotionCommand {

    // This constant is not documented.
    #[allow(missing_docs)]
    pub const CMD_STAND_UP: u8 = 1;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const CMD_SIT_DOWN: u8 = 2;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const CMD_CHANGE_GAIT: u8 = 3;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const CMD_PLAY_ANIMATION: u8 = 4;

}


impl Default for LocomotionCommand {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !hexapod_custom_msgs__msg__LocomotionCommand__init(&mut msg as *mut _) {
        panic!("Call to hexapod_custom_msgs__msg__LocomotionCommand__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for LocomotionCommand {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__LocomotionCommand__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__LocomotionCommand__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { hexapod_custom_msgs__msg__LocomotionCommand__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for LocomotionCommand {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for LocomotionCommand where Self: Sized {
  const TYPE_NAME: &'static str = "hexapod_custom_msgs/msg/LocomotionCommand";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__hexapod_custom_msgs__msg__LocomotionCommand() }
  }
}


