#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};



// Corresponds to hexapod_custom_msgs__msg__TeleopEvent

// This struct is not documented.
#[allow(missing_docs)]

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::TeleopEvent::default())
  }
}

impl rosidl_runtime_rs::Message for TeleopEvent {
  type RmwMsg = super::msg::rmw::TeleopEvent;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        event_id: msg.event_id,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
      event_id: msg.event_id,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      event_id: msg.event_id,
    }
  }
}


// Corresponds to hexapod_custom_msgs__msg__BuzzerCommand

// This struct is not documented.
#[allow(missing_docs)]

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::BuzzerCommand::default())
  }
}

impl rosidl_runtime_rs::Message for BuzzerCommand {
  type RmwMsg = super::msg::rmw::BuzzerCommand;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        command_id: msg.command_id,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
      command_id: msg.command_id,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      command_id: msg.command_id,
    }
  }
}


// Corresponds to hexapod_custom_msgs__msg__LocomotionState

// This struct is not documented.
#[allow(missing_docs)]

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::LocomotionState::default())
  }
}

impl rosidl_runtime_rs::Message for LocomotionState {
  type RmwMsg = super::msg::rmw::LocomotionState;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        state_id: msg.state_id,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
      state_id: msg.state_id,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      state_id: msg.state_id,
    }
  }
}


// Corresponds to hexapod_custom_msgs__msg__LocomotionCommand

// This struct is not documented.
#[allow(missing_docs)]

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::LocomotionCommand::default())
  }
}

impl rosidl_runtime_rs::Message for LocomotionCommand {
  type RmwMsg = super::msg::rmw::LocomotionCommand;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        command_id: msg.command_id,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
      command_id: msg.command_id,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      command_id: msg.command_id,
    }
  }
}


