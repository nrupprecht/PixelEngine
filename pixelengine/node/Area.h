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

    addSignal(&mouseLeftClicked, [this]() -> signal_emitter_t<Area*> {
      if (input::Input::IsLeftMouseJustPressed() && cursor_pos_ && pointWithinArea(*cursor_pos_)) {
        return std::make_tuple(this);
      }
      return std::nullopt;
    });

    addSignal(&mouseRightClicked, [this]() -> signal_emitter_t<Area*> {
      if (input::Input::IsRightMouseJustPressed() && cursor_pos_ && pointWithinArea(*cursor_pos_)) {
        return std::make_tuple(this);
      }
      return std::nullopt;
    });

    addSignal(&insideLeftMouseDrag, this, &Area::checkInsideLeftMouseDrag);
    addSignal(&insideRightMouseDrag, this, &Area::checkInsideRightMouseDrag);
  }

  // ==========================================================
  // Signals
  // ==========================================================

  Signal<Area*> mouseEntered;
  Signal<Area*> mouseExited;
  Signal<Area*> mouseInside;

  Signal<Area*> mouseLeftClicked;
  Signal<Area*> mouseRightClicked;

  Signal<Area*> insideLeftMouseDrag;
  Signal<Area*> insideRightMouseDrag;

protected:
  void _beginCheckSignals() override {
    // TODO: This gets the "actual position" of the cursor, not relative to the area.
    cursor_pos_        = input::Input::GetApplicationCursorPosition();
    last_mouse_inside_ = mouse_inside_;
    mouse_inside_      = cursor_pos_.has_value() && pointWithinArea(*cursor_pos_);
  }

  virtual bool pointWithinArea(const Vec2& point) const = 0;

  signal_emitter_t<Area*> checkMouseEntered() {
    const bool entered = mouse_inside_ && !last_mouse_inside_;
    if (entered) {
      return std::make_tuple(this);
    }
    return std::nullopt;
  }

  signal_emitter_t<Area*> checkMouseExited() {
    const bool exited = !mouse_inside_ && last_mouse_inside_;
    if (exited) {
      return std::make_tuple(this);
    }
    return std::nullopt;
  }

  signal_emitter_t<Area*> checkMouseInside() {
    if (mouse_inside_) {
      return std::make_tuple(this);
    }
    return std::nullopt;
  }

  virtual signal_emitter_t<Area*> checkInsideLeftMouseDrag() { return std::nullopt; }

  virtual signal_emitter_t<Area*> checkInsideRightMouseDrag() { return std::nullopt; }

  std::optional<Vec2> cursor_pos_ {};
  bool last_mouse_inside_ = false;
  bool mouse_inside_      = false;
};

}  // namespace pixelengine::node