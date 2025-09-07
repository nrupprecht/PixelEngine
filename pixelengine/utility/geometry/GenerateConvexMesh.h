#pragma once

#include "pixelengine/utility/geometry/OutlineMesh.h"
#include <span>
#include <vector>

namespace pixelengine::utility {

OutlineMesh GenerateConvexMesh(std::span<const Vec2> points);

}  // namespace pixelengine::utility