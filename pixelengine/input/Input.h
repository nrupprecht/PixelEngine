//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <ApplicationServices/ApplicationServices.h>
#include "pixelengine/utility/Vec2.h"
#include "pixelengine/utility/Signal.h"
#include <string_view>
#include <list>

namespace pixelengine::app {
  class Game; 
}

namespace pixelengine::input {

class InputSignals {
public:
  InputSignals();

  Signal<Vec2, Vec2> leftMouseDrag;
  Signal<Vec2, Vec2> rightMouseDrag;

private:
  friend class pixelengine::app::Game;

  void beginCheckSignals();
  void checkSignals();

  std::list<SignalEmitter> signals_;
};

//! \brief Class for getting input.
class Input {
public:
  struct MouseDrag {
    std::optional<Vec2> dragStart;
    std::optional<Vec2> dragEnd;
  };

  Input() = delete;

  static void Initialize();

  [[nodiscard]] static Vec2 GetCursorPosition();
  [[nodiscard]] static std::optional<Vec2> GetApplicationCursorPosition();

  static bool IsLeftMousePressed();
  static bool IsRightMousePressed();

  static bool IsLeftMouseJustPressed();
  static bool IsRightMouseJustPressed();

  static bool IsLeftMouseJustDragged();
  static bool IsRightMouseJustDragged();

  static bool IsLeftMouseJustReleased();
  static bool IsRightMouseJustReleased();

  //! \brief Get the current mouse start and current positions (which may be null, e.g. if a drag is not occurring).
  static MouseDrag GetMouseDrag(bool left_mouse);

  static bool IsPressed(int key_code);
  static bool IsJustPressed(int key_code);

  static bool IsPressed(char key);
  static bool IsJustPressed(char key);

  static bool IsPressed(std::string_view key);
  static bool IsJustPressed(std::string_view key);

  static void Update(CGRect application_frame);
  static void Checkpoint();

  static InputSignals& GetSignals();
};


}  // namespace pixelengine::input