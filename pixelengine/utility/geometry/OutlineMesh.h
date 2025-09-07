#pragma once

#include "pixelengine/utility/Vec2.h"
#include <span>
#include <vector>

namespace pixelengine::utility {

struct OutlineMesh {
  std::vector<Vec2> vertices;
  std::vector<uint16_t> indices;
};

}  // namespace pixelengine::utility