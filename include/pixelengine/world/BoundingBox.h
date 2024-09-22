//
// Created by Nathaniel Rupprecht on 9/10/24.
//

#pragma once

#include <vector>
#include <ostream>

namespace pixelengine::world {

//! \brief A structure that represents a subset of a 2D cartesian grid.
struct BoundingBox {
  BoundingBox() = default;
  BoundingBox(long long x, long long x2, long long y, long long y2)
      : x_min(x)
      , x_max(x2)
      , y_min(y)
      , y_max(y2) {}

  long long x_min = 0;
  long long x_max = -1;
  long long y_min = 0;
  long long y_max = -1;

  [[nodiscard]] bool IsEmpty() const { return x_max < x_min; }

  [[nodiscard]] bool Contains(long long x, long long y) const {
    return x_min <= x && x <= x_max && y_min <= y && y <= y_max;
  }

  void Update(long long x, long long y) {
    if (IsEmpty()) {
      x_min = x_max = x;
      y_min = y_max = y;
    }
    else {
      x_min = std::min(x, x_min);
      x_max = std::max(x, x_max);
      y_min = std::min(y, y_min);
      y_max = std::max(y, y_max);
    }
  }

  void Update(const BoundingBox& other) {
    if (other.IsEmpty()) {
      return;
    }
    if (IsEmpty()) {
      *this = other;
      return;
    }
    x_min = std::min(x_min, other.x_min);
    x_max = std::max(x_max, other.x_max);
    y_min = std::min(y_min, other.y_min);
    y_max = std::max(y_max, other.y_max);
  }

  void Expand(long long amount) {
    if (x_min <= x_max) {
      x_min -= amount;
      x_max += amount;
      y_min -= amount;
      y_max += amount;
    }
  }

  std::array<long long, 4> Clip(long long width, long long height) {
    auto _x_min = std::max(0ll, x_min);
    auto _x_max = std::min(width - 1, x_max);
    auto _y_min = std::max(0ll, y_min);
    auto _y_max = std::min(height - 1, y_max);
    return {_x_min, _x_max, _y_min, _y_max};
  }

  friend std::ostream& operator<<(std::ostream& out, const BoundingBox& bb) {
    out << "(" << bb.x_min << ", " << bb.y_min << ") x (" << bb.x_max << ", " << bb.y_max << ")";
    return out;
  }
};

}