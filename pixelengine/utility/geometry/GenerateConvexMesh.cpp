#include "pixelengine/utility/geometry/GenerateConvexMesh.h"
#include "pixelengine/utility/Contracts.h"

namespace pixelengine::utility {

OutlineMesh GenerateConvexMesh(std::span<const Vec2> points) {
  PIXEL_ASSERT(points.size() >= 3, "Convex mesh must have at least 3 points");

  OutlineMesh mesh;
  mesh.vertices.assign(points.begin(), points.end());
  mesh.indices.reserve((points.size() - 2) * 3);

  // Fan triangulation: (0, i, i+1)
  for (uint16_t i = 1; i < points.size() - 1; ++i) {
    mesh.indices.push_back(0);
    mesh.indices.push_back(i);
    mesh.indices.push_back(i + 1);
  }

  return mesh;
}

}  // namespace pixelengine::utility