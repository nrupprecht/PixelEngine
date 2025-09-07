#pragma once

#include "pixelengine/node/Node.h"
#include "pixelengine/utility/Signal.h"
#include "pixelengine/utility/Vec2.h"

namespace pixelengine::node {

//! \brief A point node that can be used to detect mouse events.
class Point : public Node {
public:
  Point(Vec2 position) : position_(position) {
    addSignal(&insideLeftMouseDrag, [this]() -> signal_emitter_t<Point*> {
      if (input::Input::IsLeftMouseJustDragged()) {
        auto [start, end] = input::Input::GetMouseDrag(true /* true => left */);
        if (WithinRectangle(*start, *end, position_)) {
          return std::make_tuple(this);
        }
      }
      return {};
    });

    addSignal(&insideRightMouseDrag, [this]() -> signal_emitter_t<Point*> {
      if (input::Input::IsRightMouseJustDragged()) {
        auto [start, end] = input::Input::GetMouseDrag(false /* false => right */);
        if (WithinRectangle(*start, *end, position_)) {
          return std::make_tuple(this);
        }
      }
      return {};
    });
  }

  Signal<Point*> insideLeftMouseDrag;
  Signal<Point*> insideRightMouseDrag;

  Vec2 GetPosition() const { return position_; }

protected:
  void _beginCheckSignals() override {
    // TODO: This gets the "actual position" of the cursor, not relative to the area.
    cursor_pos_ = input::Input::GetApplicationCursorPosition();
  }

  NO_DISCARD Vec2 _additionalOffset() const override { return position_; }

  Vec2 position_;

  std::optional<Vec2> cursor_pos_ {};
};

}  // namespace pixelengine::node