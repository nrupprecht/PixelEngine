//
// Created by Nathaniel Rupprecht on 9/26/24.
//

#pragma once

#include "pixelengine/utility/Vec2.h"

namespace pixelengine::application {

struct WindowContext {
  float coord_left{0.f}, coord_right{1.f}, coord_bottom{0.f}, coord_top{1.f};

  [[nodiscard]] Vec2 TranslatePoint(Vec2 coords) const {
    // The screen coordinates are [-1, 1] x [-1, 1]
    return Vec2 {
      2.f * (coords.x - coord_left) / (coord_right - coord_left) - 1.f,
      2.f * (coords.y - coord_bottom) / (coord_top - coord_bottom) - 1.f
    };
  }
};

}  // namespace pixelengine::application