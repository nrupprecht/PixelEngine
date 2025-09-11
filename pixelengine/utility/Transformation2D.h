#pragma once

#include "pixelengine/utility/Mat2.h"
#include <simd/simd.h>

namespace pixelengine::math {

struct Transformation2D {
  Vec2 displacement;
  Mat2 transformation;

  //! \brief Compose two transformations.
  friend Transformation2D operator*(const Transformation2D& lhs, const Transformation2D& rhs) {
    return {lhs.transformation * rhs.displacement + lhs.displacement, lhs.transformation * rhs.transformation};
  }

  Vec2 TransformPoint(const Vec2& point) const {
    return transformation * point + displacement;
  }

  simd::float3 TransformPoint(const simd::float3& point) const {
    auto p = TransformPoint(Vec2{point.x, point.y});
    return {p.x, p.y, point.z};
  }

  static Transformation2D Identity() {
    return {Vec2{0, 0}, Mat2::Identity()};
  }

  friend std::ostream& operator<<(std::ostream& os, const Transformation2D& t) {
    os << "[[" << t.transformation(0, 0) << ", " << t.transformation(0, 1) << "], [" << t.transformation(1, 0)
       << ", " << t.transformation(1, 1) << "]] + (" << t.displacement.x << ", " << t.displacement.y << ")";
    return os;
  }

};

}  // namespace pixelengine::math