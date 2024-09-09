//
// Created by Nathaniel Rupprecht on 9/8/24.
//

#include <iostream>

#include "pixelengine/input/Input.h"

namespace pixelengine::input {

//! \brief Structure for keeping track of the mouse states.
struct MouseStates {
  bool left_mouse_down       = false;
  bool left_mouse_just_down  = false;
  bool right_mouse_down      = false;
  bool right_mouse_just_down = false;

  bool left_mouse_down_last_checkpoint  = false;
  bool right_mouse_down_last_checkpoint = false;

  CGPoint cursor_position{};

  void Update() {
    CGEventRef event = CGEventCreate(nullptr);
    cursor_position  = CGEventGetLocation(event);
    CFRelease(event);
  }

  void Checkpoint() {
    left_mouse_down_last_checkpoint = left_mouse_down;
    // left_mouse_down                 = false;
    left_mouse_just_down = false;

    right_mouse_down_last_checkpoint = right_mouse_down;
    // right_mouse_down                 = false;
    right_mouse_just_down = false;
  }
};

//! \brief Structure for keeping track of keyboard states.
struct KeyStates {
  struct State {
    bool is_pressed                  = false;
    bool is_just_pressed             = false;
    bool was_pressed_last_checkpoint = false;
    bool was_shift_on                = false;
    bool was_caps_on                 = false;
  };

  State states[127];
  bool caps_on         = false;
  bool shift_depressed = false;

  CGEventFlags last_flags = 0;

  void Checkpoint() {
    for (auto& state : states) {
      state.was_pressed_last_checkpoint = state.is_pressed;
      state.is_just_pressed             = false;

      state.is_pressed = false;

      // What else?
    }
  }
};

// Key state singleton.
KeyStates _key_states {};

MouseStates _mouse_states {};


namespace {

// The following method converts the key code returned by each keypress as a human readable key code in const
// char format.
const char* convertKeyCode(int key_code, bool shift, bool caps) {
  switch (key_code) {
    default:
      throw std::runtime_error("impossible key code");
    case 0:
      return shift || caps ? "A" : "a";
    case 1:
      return shift || caps ? "S" : "s";
    case 2:
      return shift || caps ? "D" : "d";
    case 3:
      return shift || caps ? "F" : "f";
    case 4:
      return shift || caps ? "H" : "h";
    case 5:
      return shift || caps ? "G" : "g";
    case 6:
      return shift || caps ? "Z" : "z";
    case 7:
      return shift || caps ? "X" : "x";
    case 8:
      return shift || caps ? "C" : "c";
    case 9:
      return shift || caps ? "V" : "v";
    case 11:
      return shift || caps ? "B" : "b";
    case 12:
      return shift || caps ? "Q" : "q";
    case 13:
      return shift || caps ? "W" : "w";
    case 14:
      return shift || caps ? "E" : "e";
    case 15:
      return shift || caps ? "R" : "r";
    case 16:
      return shift || caps ? "Y" : "y";
    case 17:
      return shift || caps ? "T" : "t";
    case 18:
      return shift ? "!" : "1";
    case 19:
      return shift ? "@" : "2";
    case 20:
      return shift ? "#" : "3";
    case 21:
      return shift ? "$" : "4";
    case 22:
      return shift ? "^" : "6";
    case 23:
      return shift ? "%" : "5";
    case 24:
      return shift ? "+" : "=";
    case 25:
      return shift ? "(" : "9";
    case 26:
      return shift ? "&" : "7";
    case 27:
      return shift ? "_" : "-";
    case 28:
      return shift ? "*" : "8";
    case 29:
      return shift ? ")" : "0";
    case 30:
      return shift ? "}" : "]";
    case 31:
      return shift || caps ? "O" : "o";
    case 32:
      return shift || caps ? "U" : "u";
    case 33:
      return shift ? "{" : "[";
    case 34:
      return shift || caps ? "I" : "i";
    case 35:
      return shift || caps ? "P" : "p";
    case 37:
      return shift || caps ? "L" : "l";
    case 38:
      return shift || caps ? "J" : "j";
    case 39:
      return shift ? "\"" : "'";
    case 40:
      return shift || caps ? "K" : "k";
    case 41:
      return shift ? ":" : ";";
    case 42:
      return shift ? "|" : "\\";
    case 43:
      return shift ? "<" : ",";
    case 44:
      return shift ? "?" : "/";
    case 45:
      return shift || caps ? "N" : "n";
    case 46:
      return shift || caps ? "M" : "m";
    case 47:
      return shift ? ">" : ".";
    case 50:
      return shift ? "~" : "`";
    case 65:
      return "[decimal]";
    case 67:
      return "[asterisk]";
    case 69:
      return "[plus]";
    case 71:
      return "[clear]";
    case 75:
      return "[divide]";
    case 76:
      return "[enter]";
    case 78:
      return "[hyphen]";
    case 81:
      return "[equals]";
    case 82:
      return "0";
    case 83:
      return "1";
    case 84:
      return "2";
    case 85:
      return "3";
    case 86:
      return "4";
    case 87:
      return "5";
    case 88:
      return "6";
    case 89:
      return "7";
    case 91:
      return "8";
    case 92:
      return "9";
    case 36:
      return "[return]";
    case 48:
      return "[tab]";
    case 49:
      return " ";
    case 51:
      return "[del]";
    case 53:
      return "[esc]";
    case 54:
      return "[right-cmd]";
    case 55:
      return "[left-cmd]";
    case 56:
      return "[left-shift]";
    case 57:
      return "[caps]";
    case 58:
      return "[left-option]";
    case 59:
      return "[left-ctrl]";
    case 60:
      return "[right-shift]";
    case 61:
      return "[right-option]";
    case 62:
      return "[right-ctrl]";
    case 63:
      return "[fn]";
    case 64:
      return "[f17]";
    case 72:
      return "[volup]";
    case 73:
      return "[voldown]";
    case 74:
      return "[mute]";
    case 79:
      return "[f18]";
    case 80:
      return "[f19]";
    case 90:
      return "[f20]";
    case 96:
      return "[f5]";
    case 97:
      return "[f6]";
    case 98:
      return "[f7]";
    case 99:
      return "[f3]";
    case 100:
      return "[f8]";
    case 101:
      return "[f9]";
    case 103:
      return "[f11]";
    case 105:
      return "[f13]";
    case 106:
      return "[f16]";
    case 107:
      return "[f14]";
    case 109:
      return "[f10]";
    case 111:
      return "[f12]";
    case 113:
      return "[f15]";
    case 114:
      return "[help]";
    case 115:
      return "[home]";
    case 116:
      return "[pgup]";
    case 117:
      return "[fwddel]";
    case 118:
      return "[f4]";
    case 119:
      return "[end]";
    case 120:
      return "[f2]";
    case 121:
      return "[pgdown]";
    case 122:
      return "[f1]";
    case 123:
      return "[left]";
    case 124:
      return "[right]";
    case 125:
      return "[down]";
    case 126:
      return "[up]";
  }
}

CGEventRef mouseCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon) {
  // Mouse events.
  if (type == kCGEventLeftMouseDown) {
    _mouse_states.left_mouse_down      = true;
    _mouse_states.left_mouse_just_down = true;
  }
  else if (type == kCGEventLeftMouseUp) {
    _mouse_states.left_mouse_down      = false;
    _mouse_states.left_mouse_just_down = false;
  }
  else if (type == kCGEventRightMouseDown) {
    _mouse_states.right_mouse_down      = true;
    _mouse_states.right_mouse_just_down = true;
  }
  else if (type == kCGEventRightMouseUp) {
    _mouse_states.right_mouse_down      = false;
    _mouse_states.right_mouse_just_down = false;
  }

  return event;
}

CGEventRef keyCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon) {
  // Good reference: https://github.com/caseyscarborough/keylogger

  // Filter out to only include key down events.
  if (type != kCGEventKeyDown && type != kCGEventFlagsChanged) {
    return event;
  }

  CGEventFlags flags = CGEventGetFlags(event);

  // Retrieve the incoming keycode.
  auto key_code = static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));

  // Calculate key up/down.
  bool down       = false;
  auto last_flags = _key_states.last_flags;
  if (type == kCGEventFlagsChanged) {
    switch (key_code) {
      case 54:  // [right-cmd]
      case 55:  // [left-cmd]
        down = (flags & kCGEventFlagMaskCommand) && !(last_flags & kCGEventFlagMaskCommand);
        break;
      case 56:  // [left-shift]
      case 60:  // [right-shift]
        down = (flags & kCGEventFlagMaskShift) && !(last_flags & kCGEventFlagMaskShift);
        break;
      case 58:  // [left-option]
      case 61:  // [right-option]
        down = (flags & kCGEventFlagMaskAlternate) && !(last_flags & kCGEventFlagMaskAlternate);
        break;
      case 59:  // [left-ctrl]
      case 62:  // [right-ctrl]
        down = (flags & kCGEventFlagMaskControl) && !(last_flags & kCGEventFlagMaskControl);
        break;
      case 57:  // [caps]
        down = (flags & kCGEventFlagMaskAlphaShift) && !(last_flags & kCGEventFlagMaskAlphaShift);
        break;
      default:
        break;
    }
  }
  else if (type == kCGEventKeyDown) {
    down = true;
  }

  // Only log key down events.
  if (!down) {
    return event;
  }

  _key_states.shift_depressed = flags & kCGEventFlagMaskShift;
  _key_states.caps_on         = flags & kCGEventFlagMaskAlphaShift;

  auto& state           = _key_states.states[key_code];
  state.is_just_pressed = !state.was_pressed_last_checkpoint;
  state.is_pressed      = true;

  // Update flags.
  _key_states.last_flags = flags;

  // std::cout << convertKeyCode(keyCode, shift, caps) << std::endl;
  return event;
}

void setMouseEvents() {
  auto mouse_event_mask = CGEventMaskBit(kCGEventLeftMouseDown) | CGEventMaskBit(kCGEventLeftMouseUp)
      | CGEventMaskBit(kCGEventRightMouseDown) | CGEventMaskBit(kCGEventRightMouseUp);

  CFMachPortRef mouse_event = CGEventTapCreate(kCGSessionEventTap,
                                               kCGHeadInsertEventTap,
                                               kCGEventTapOptionListenOnly,  // kCGEventTapOptionDefault,
                                               mouse_event_mask,
                                               mouseCallback,
                                               nullptr);
  // Exit the program if unable to create the event tap.
  if (!mouse_event) {
    fprintf(stderr, "ERROR: Unable to create event tap.\n");
    exit(1);
  }

  // Create a run loop source and add enable the event tap.
  CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, mouse_event, 0);
  CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
  CGEventTapEnable(mouse_event, true);
}

void setKeyEvents() {
  // Create an event tap to retrieve keypresses.
  auto key_event_mask     = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);
  CFMachPortRef key_event = CGEventTapCreate(kCGSessionEventTap,
                                             kCGHeadInsertEventTap,
                                             kCGEventTapOptionListenOnly,  // kCGEventTapOptionDefault,
                                             key_event_mask,
                                             keyCallback,
                                             nullptr);
  // Exit the program if unable to create the event tap.
  if (!key_event) {
    fprintf(stderr, "ERROR: Unable to create event tap.\n");
    exit(1);
  }

  // Create a run loop source and add enable the event tap.
  CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, key_event, 0);
  CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
  CGEventTapEnable(key_event, true);
}

}  // namespace

void Input::Initialize() {
  setMouseEvents();
  setKeyEvents();
}

CGPoint Input::GetCursorPosition() {
  return _mouse_states.cursor_position;
}

bool Input::IsLeftMousePressed() {
  return _mouse_states.left_mouse_down;
}

bool Input::IsRightMousePressed() {
  return _mouse_states.right_mouse_down;
}

bool Input::IsLeftMouseJustPressed() {
  return _mouse_states.left_mouse_just_down;
}

bool Input::IsRightMouseJustPressed() {
  return _mouse_states.right_mouse_just_down;
}

bool Input::IsPressed(int key_code) {
  assert(key_code < 127);
  return _key_states.states[key_code].is_pressed;
}

bool Input::IsJustPressed(int key_code) {
  assert(key_code < 127);
  return _key_states.states[key_code].is_just_pressed;
}

void Input::Update() {
  _mouse_states.Update();
}

void Input::Checkpoint() {
  _key_states.Checkpoint();
  _mouse_states.Checkpoint();
}


}  // namespace pixelengine::input
