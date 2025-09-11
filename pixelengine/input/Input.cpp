//
// Created by Nathaniel Rupprecht on 9/8/24.
//

#include "pixelengine/input/Input.h"
// Other files.
#include "pixelengine/utility/Contracts.h"
#include "pixelengine/utility/Utility.h"

namespace pixelengine::input {

namespace {

std::optional<Vec2> getApplicationCursorPosition(CGRect frame) {
  // Y measured from top left.
  auto cursor          = input::Input::GetCursorPosition();
  auto [width, height] = GetScreenResolution();
  cursor.y             = height - cursor.y;

  auto origin = frame.origin;
  auto size   = frame.size;

  // TODO: The window's upper bar is 28 pixels tall. How to tell this in general?
  size.height -= 28;

  if (origin.x <= cursor.x                 //
      && cursor.x < origin.x + size.width  //
      && origin.y <= cursor.y              //
      && cursor.y < origin.y + size.height)
  {
    // Normalize cursor position.
    auto x = static_cast<float>((cursor.x - origin.x) / size.width);
    auto y = static_cast<float>((cursor.y - origin.y) / size.height);

    // Translate into [-1, 1] coordinates.
    x = 2 * (x - 0.5);
    y = 2 * (y - 0.5);

    return Vec2 {x, y};
  }
  return {};
}

}  // namespace

//! \brief Structure for keeping track of the mouse states.
struct MouseStates {
  using mouse_pos_t = Vec2;

  bool left_mouse_down          = false;
  bool left_mouse_just_down     = false;
  bool left_mouse_just_released = false;
  bool left_mouse_just_dragged  = false;

  bool right_mouse_down          = false;
  bool right_mouse_just_down     = false;
  bool right_mouse_just_released = false;
  bool right_mouse_just_dragged  = false;

  bool left_mouse_down_last_checkpoint  = false;
  bool right_mouse_down_last_checkpoint = false;

  CGPoint cursor_position {};

  std::optional<mouse_pos_t> last_application_cursor_position {};
  std::optional<mouse_pos_t> application_cursor_position {};

  //! \brief The position where the mouse was last pressed down.
  std::optional<mouse_pos_t> left_mouse_down_position {};
  std::optional<mouse_pos_t> right_mouse_down_position {};

  void Update(CGRect application_frame) {
    CGEventRef event                 = CGEventCreate(nullptr);
    cursor_position                  = CGEventGetLocation(event);
    last_application_cursor_position = application_cursor_position;
    application_cursor_position      = getApplicationCursorPosition(application_frame);
    CFRelease(event);

    if (left_mouse_just_down) {
      left_mouse_down_position = application_cursor_position;
    }
    if (!left_mouse_down && left_mouse_down_last_checkpoint) {
      left_mouse_just_released = true;
    }

    if (right_mouse_just_down) {
      right_mouse_down_position = application_cursor_position;
    }
    else if (!right_mouse_down && right_mouse_down_last_checkpoint) {
      right_mouse_just_released = true;
    }

    if (left_mouse_just_released) {
      left_mouse_just_dragged = left_mouse_just_released && application_cursor_position
          && left_mouse_down_position && *left_mouse_down_position != *application_cursor_position;
    }
    if (right_mouse_just_released) {
      right_mouse_just_dragged = right_mouse_just_released && application_cursor_position
          && right_mouse_down_position && *right_mouse_down_position != *application_cursor_position;
    }
  }

  void Checkpoint() {
    left_mouse_down_last_checkpoint = left_mouse_down;
    left_mouse_just_down            = false;
    left_mouse_just_released        = false;
    left_mouse_just_dragged         = false;

    right_mouse_down_last_checkpoint = right_mouse_down;
    right_mouse_just_down            = false;
    right_mouse_just_released        = false;
    right_mouse_just_dragged         = false;

    if (!left_mouse_down) {
      left_mouse_down_position = {};
    }
    if (!right_mouse_down) {
      right_mouse_down_position = {};
    }
  }
};

//! \brief Structure for keeping track of keyboard states.
struct KeyStates {
  struct State {
    bool is_pressed       = false;
    bool is_just_pressed  = false;
    bool is_just_released = false;

    //! \brief Whether the key has been released since the last check point
    bool un_press_queued = false;

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
      state.is_just_released            = false;
      state.is_pressed                  = state.is_pressed && !state.un_press_queued;

      // TODO: Handle shift, caps.
    }
  }
};

namespace {

// Key state singleton.
KeyStates _key_states {};

MouseStates _mouse_states {};

InputSignals _signals {};

}  // namespace


namespace {

// The following method converts the key code returned by each keypress as a human readable key code in const
// char format.
[[maybe_unused]] const char* convertKeyCode(int key_code, bool shift, bool caps) {
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

[[maybe_unused]] uint8_t toKeyCode(std::string_view key) {
  static const std::unordered_map<std::string_view, int> _key_map =  //
      {{"a", 0},
       {"s", 1},
       {"d", 2},
       {"f", 3},
       {"h", 4},
       {"g", 5},
       {"z", 6},
       {"x", 7},
       {"c", 8},
       {"v", 9},
       {"b", 11},
       {"q", 12},
       {"w", 13},
       {"e", 14},
       {"r", 15},
       {"y", 16},
       {"t", 17},
       {"1", 18},
       {"2", 19},
       {"3", 20},
       {"4", 21},
       {"6", 22},
       {"5", 23},
       {"=", 24},
       {"9", 25},
       {"7", 26},
       {"-", 27},
       {"8", 28},
       {"0", 29},
       {"]", 30},
       {"o", 31},
       {"u", 32},
       {"[", 33},
       {"i", 34},
       {"p", 35},
       {"l", 37},
       {"j", 38},
       {"'", 39},
       {"k", 40},
       {";", 41},
       {"\\", 42},
       {",", 43},
       {"/", 44},
       {"n", 45},
       {"m", 46},
       {".", 47},
       {"`", 50},
       {"[return]", 36},
       {"[tab]", 48},
       {" ", 49},
       {"[del]", 51},
       {"[esc]", 53},
       {"[right-cmd]", 54},
       {"[left-cmd]", 55},
       {"[left-shift]", 56},
       {"[caps]", 57},
       {"[left-option]", 58},
       {"[left-ctrl]", 59},
       {"[right-shift]", 60},
       {"[right-option]", 61},
       {"[right-ctrl]", 62},
       {"[fn]", 63},
       {"[f17]", 64},
       {"[volup]", 72},
       {"[voldown]", 73},
       {"[mute]", 74},
       {"[f18]", 79},
       {"[f19]", 80},
       {"[f20]", 90},
       {"[f5]", 96},
       {"[f6]", 97},
       {"[f7]", 98},
       {"[f3]", 99},
       {"[f8]", 100},
       {"[f9]", 101},
       {"[f11]", 103},
       {"[f13]", 105},
       {"[f16]", 106},
       {"[f14]", 107},
       {"[f10]", 109},
       {"[f12]", 111},
       {"[f15]", 113},
       {"[help]", 114},
       {"[home]", 115},
       {"[pgup]", 116},
       {"[fwddel]", 117},
       {"[f4]", 118},
       {"[end]", 119},
       {"[f2]", 120},
       {"[pgdown]", 121},
       {"[f1]", 122},
       {"[left]", 123},
       {"[right]", 124},
       {"[down]", 125},
       {"[up]", 126},
       {"A", 0},
       {"S", 1},
       {"D", 2},
       {"F", 3},
       {"H", 4},
       {"G", 5},
       {"Z", 6},
       {"X", 7},
       {"C", 8},
       {"V", 9},
       {"B", 11},
       {"Q", 12},
       {"W", 13},
       {"E", 14},
       {"R", 15},
       {"Y", 16},
       {"T", 17},
       {"O", 31},
       {"U", 32},
       {"I", 34},
       {"P", 35},
       {"L", 37},
       {"J", 38},
       {"K", 40},
       {"N", 45},
       {"M", 46}};

  if (auto it = _key_map.find(key); it != _key_map.end()) {
    return it->second;
  }
  return {};
}


CGEventRef mouseCallback([[maybe_unused]] CGEventTapProxy proxy,
                         CGEventType type,
                         CGEventRef event,
                         [[maybe_unused]] void* refcon) {
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

CGEventRef keyCallback([[maybe_unused]] CGEventTapProxy proxy,
                       CGEventType type,
                       CGEventRef event,
                       [[maybe_unused]] void* refcon) {
  // Good reference: https://github.com/caseyscarborough/keylogger

  // Filter out to only include key down events.
  if (type != kCGEventKeyDown && type != kCGEventFlagsChanged && type != kCGEventKeyUp) {
    return event;
  }

  const CGEventFlags flags = CGEventGetFlags(event);

  // Retrieve the incoming keycode.
  const auto key_code = static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));

  // Calculate key up/down.
  bool up               = false;
  bool down             = false;
  const auto last_flags = _key_states.last_flags;
  if (type == kCGEventKeyUp) {
    up = true;
  }
  else if (type == kCGEventFlagsChanged) {
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
  if (!down && !up) {
    return event;
  }

  _key_states.shift_depressed = flags & kCGEventFlagMaskShift;
  _key_states.caps_on         = flags & kCGEventFlagMaskAlphaShift;

  auto& state           = _key_states.states[key_code];
  state.is_just_pressed = !state.was_pressed_last_checkpoint;
  state.is_pressed      = true;
  // If the key is released, queue a release.
  state.un_press_queued = up;

  // Update flags.
  _key_states.last_flags = flags;

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
  auto key_event_mask =
      CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp) | CGEventMaskBit(kCGEventFlagsChanged);
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

InputSignals::InputSignals() {
  // Initialize signals.

  signal_check_t<Vec2, Vec2> leftMouseDragCheck = []() -> signal_emitter_t<Vec2, Vec2> {
    if (_mouse_states.left_mouse_just_dragged) {
      return std::make_tuple(*_mouse_states.left_mouse_down_position,
                             *_mouse_states.application_cursor_position);
    }
    return std::nullopt;
  };
  signals_.emplace_back(&leftMouseDrag, leftMouseDragCheck);

  signal_check_t<Vec2, Vec2> rightMouseDragCheck = []() -> signal_emitter_t<Vec2, Vec2> {
    if (_mouse_states.right_mouse_just_dragged) {
      return std::make_tuple(*_mouse_states.right_mouse_down_position,
                             *_mouse_states.application_cursor_position);
    }
    return std::nullopt;
  };
  signals_.emplace_back(&rightMouseDrag, rightMouseDragCheck);
}

void InputSignals::beginCheckSignals() {}

void InputSignals::checkSignals() {
  for (auto& signal : signals_) {
    signal.CheckSignal();
  }
}

void Input::Initialize() {
  setMouseEvents();
  setKeyEvents();
}

Vec2 Input::GetCursorPosition() {
  return Vec2(_mouse_states.cursor_position.x, _mouse_states.cursor_position.y);
}

std::optional<Vec2> Input::GetApplicationCursorPosition() {
  return _mouse_states.application_cursor_position;
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

bool Input::IsLeftMouseJustDragged() {
  return _mouse_states.left_mouse_just_dragged;
}

bool Input::IsRightMouseJustDragged() {
  return _mouse_states.right_mouse_just_dragged;
}

bool Input::IsLeftMouseJustReleased() {
  return _mouse_states.left_mouse_just_released;
}

bool Input::IsRightMouseJustReleased() {
  return _mouse_states.right_mouse_just_released;
}

Input::MouseDrag Input::GetMouseDrag(bool left_mouse) {
  return {left_mouse ? _mouse_states.left_mouse_down_position : _mouse_states.right_mouse_down_position,
          _mouse_states.application_cursor_position};
}

bool Input::IsPressed(int key_code) {
  PIXEL_REQUIRE(key_code < 127, "key code must be < 127, was " << key_code);
  return _key_states.states[key_code].is_pressed;
}

bool Input::IsJustPressed(int key_code) {
  PIXEL_REQUIRE(key_code < 127, "key code must be < 127, was " << key_code);
  return _key_states.states[key_code].is_just_pressed;
}

bool Input::IsPressed(char key) {
  std::string_view sv {&key, 1};
  return IsPressed(sv);
}

bool Input::IsJustPressed(char key) {
  std::string_view sv {&key, 1};
  return IsJustPressed(sv);
}

bool Input::IsPressed(std::string_view key) {
  return IsPressed(toKeyCode(key));
}

bool Input::IsJustPressed(std::string_view key) {
  return IsJustPressed(toKeyCode(key));
}

void Input::Update(CGRect application_frame) {
  _mouse_states.Update(application_frame);
}

void Input::Checkpoint() {
  _key_states.Checkpoint();
  _mouse_states.Checkpoint();
}

InputSignals& Input::GetSignals() {
  return _signals;
}


}  // namespace pixelengine::input
