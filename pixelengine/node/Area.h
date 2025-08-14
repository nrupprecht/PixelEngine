#pragma once

#include "pixelengine/node/Node.h"
#include "pixelengine/utility/Signal.h"
#include "pixelengine/utility/Vec2.h"

namespace pixelengine::node {

class Area : public Node {
public:
  Area() {
    addSignal(&mouseEntered, this, &Area::checkMouseEntered);
    addSignal(&mouseExited, this, &Area::checkMouseExited);
    addSignal(&mouseInside, this, &Area::checkMouseInside);
  }

  Signal<> mouseEntered;
  Signal<> mouseExited;
  Signal<> mouseInside;

protected:
  void _beginCheckSignals() override {
    // TODO: This gets the "actual position" of the cursor, not relative to the area.
    cursor_pos_        = input::Input::GetApplicationCursorPosition();
    last_mouse_inside_ = mouse_inside_;
    mouse_inside_      = cursor_pos_.has_value() && pointWithinArea(*cursor_pos_);
  }

  virtual bool pointWithinArea(const Vec2& point) const = 0;

  std::optional<std::tuple<>> checkMouseEntered() {
    const bool entered = mouse_inside_ && !last_mouse_inside_;
    if (entered) {
      return std::make_tuple();
    }
    return std::nullopt;
  }

  std::optional<std::tuple<>> checkMouseExited() {
    const bool exited = !mouse_inside_ && last_mouse_inside_;
    if (exited) {
      return std::make_tuple();
    }
    return std::nullopt;
  }

  std::optional<std::tuple<>> checkMouseInside() {
    if (mouse_inside_) {
      return std::make_tuple();
    }
    return std::nullopt;
  }

  std::optional<Vec2> cursor_pos_ {};
  bool last_mouse_inside_ = false;
  bool mouse_inside_      = false;
};

}  // namespace pixelengine::node