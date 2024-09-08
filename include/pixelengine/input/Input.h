//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <ApplicationServices/ApplicationServices.h>
#include <CoreGraphics/CGEventTypes.h>

namespace pixelengine::input {

// CGEventRef CGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
//   if (type != kCGEventKeyDown && type != kCGEventFlagsChanged) {
//     return event;
//   }
//
//   CGEventFlags flags = CGEventGetFlags(event);
//
//   // Retrieve the incoming keycode.
//   CGKeyCode keyCode = (CGKeyCode) CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
//
//   // Calculate key up/down.
//   bool down = false;
//   if (type == kCGEventFlagsChanged) {
//     switch (keyCode) {
//       case 54: // [right-cmd]
//       case 55: // [left-cmd]
//           down = (flags & kCGEventFlagMaskCommand) && !(lastFlags & kCGEventFlagMaskCommand);
//       break;
//       case 56: // [left-shift]
//       case 60: // [right-shift]
//           down = (flags & kCGEventFlagMaskShift) && !(lastFlags & kCGEventFlagMaskShift);
//       break;
//       case 58: // [left-option]
//       case 61: // [right-option]
//           down = (flags & kCGEventFlagMaskAlternate) && !(lastFlags & kCGEventFlagMaskAlternate);
//       break;
//       case 59: // [left-ctrl]
//       case 62: // [right-ctrl]
//           down = (flags & kCGEventFlagMaskControl) && !(lastFlags & kCGEventFlagMaskControl);
//       break;
//       case 57: // [caps]
//         down = (flags & kCGEventFlagMaskAlphaShift) && !(lastFlags & kCGEventFlagMaskAlphaShift);
//       break;
//       default:
//         break;
//     }
//   } else if (type == kCGEventKeyDown) {
//     down = true;
//   }
//   lastFlags = flags;
//
//   // Only log key down events.
//   if (!down) {
//     return event;
//   }
//
//   // Print the human readable key to the logfile.
//   bool shift = flags & kCGEventFlagMaskShift;
//   bool caps = flags & kCGEventFlagMaskAlphaShift;
//   // fprintf(logfile, "%s", convertKeyCode(keyCode, shift, caps));
//   // fflush(logfile);
//   return event;
// }


class Input {
  Input() = default;

  void update() {
    CGEventRef event = CGEventCreate(nullptr);
    cursor_position_ = CGEventGetLocation(event);
    CFRelease(event);

    CGEventRef mkey = CGEventCreateKeyboardEvent(nullptr, (CGKeyCode)46, true);
    CGEventPost(kCGAnnotatedSessionEventTap, mkey);
    CFRelease(mkey);
  }

  CGPoint cursor_position_;
};


}  // namespace pixelengine::input