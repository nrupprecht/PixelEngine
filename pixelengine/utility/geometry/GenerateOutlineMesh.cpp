#include <cmath>
#include <cstdint>
#include <span>
#include <vector>

#include "pixelengine/utility/geometry/GenerateOutlineMesh.h"


namespace pixelengine::utility {

inline Vec2 perpendicular(const Vec2& v) {
  // Clockwise polygon, so we take the outward normal
  return {v.y, -v.x};
}

OutlineMesh GenerateOutlineMesh(std::span<const Vec2> points, float num_pixels, bool closed) {
  OutlineMesh mesh;
  if (points.size() < 2) return mesh;

  float half_width = num_pixels * 0.5f;
  size_t count     = points.size();

  std::vector<Vec2> left(count), right(count);

  // Compute left/right offsets
  for (size_t i = 0; i < count; i++) {
    Vec2 prev = points[(i == 0) ? (closed ? count - 1 : 0) : i - 1];
    Vec2 curr = points[i];
    Vec2 next = points[(i + 1 == count) ? (closed ? 0 : count - 1) : i + 1];

    Vec2 dir_prev = normalize(curr - prev);
    Vec2 dir_next = normalize(next - curr);

    Vec2 n_prev = perpendicular(dir_prev);
    Vec2 n_next = perpendicular(dir_next);

    if (!closed && (i == 0 || i == count - 1)) {
      // Start/end point for open path — just offset using the segment normal
      Vec2 normal = (i == 0) ? n_next : n_prev;
      left[i]     = curr + normal * half_width;
      right[i]    = curr - normal * half_width;
    }
    else {
      // Mitre join for internal or closed path points
      Vec2 bisector   = normalize(n_prev + n_next);
      float mitre_len = half_width / std::max(0.1f, bisector * n_prev);

      left[i]  = curr + bisector * mitre_len;
      right[i] = curr - bisector * mitre_len;
    }
  }

  // Build vertex list
  for (size_t i = 0; i < count; i++) {
    mesh.vertices.push_back(left[i]);
    mesh.vertices.push_back(right[i]);
  }

  // Build index list
  size_t seg_count = closed ? count : count - 1;
  for (size_t i = 0; i < seg_count; i++) {
    uint16_t i0 = static_cast<uint16_t>((i * 2) % (count * 2));
    uint16_t i1 = static_cast<uint16_t>((i * 2 + 1) % (count * 2));
    uint16_t i2 = static_cast<uint16_t>(((i + 1) * 2) % (count * 2));
    uint16_t i3 = static_cast<uint16_t>(((i + 1) * 2 + 1) % (count * 2));

    mesh.indices.push_back(i0);
    mesh.indices.push_back(i2);
    mesh.indices.push_back(i1);

    mesh.indices.push_back(i1);
    mesh.indices.push_back(i2);
    mesh.indices.push_back(i3);
  }

  return mesh;
}


}  // namespace pixelengine::utility