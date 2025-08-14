#pragma once

#include "pixelengine/utility/Vec2.h"
#include <span>
#include <vector>

namespace pixelengine::utility {

struct OutlineMesh {
  std::vector<Vec2> vertices;
  std::vector<uint16_t> indices;
};

OutlineMesh GenerateOutlineMesh(std::span<const Vec2> points, float numPixels, bool closed = true);

}  // namespace pixelengine::utility