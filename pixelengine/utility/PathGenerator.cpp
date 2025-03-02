//
// Created by Nathaniel Rupprecht on 10/6/24.
//

#include "pixelengine/utility/PathGenerator.h"
// Other files.

namespace pixelengine {

PathGenerator::PathGenerator(PVec2 start, PVec2 end) : start_(start), end_(end), next_(start) {
  dx_ = end_.x - start_.x;
  dy_ = end_.y - start_.y;

  sx = start_.x < end_.x ? 1 : -1;
  sy = start_.y < end_.y ? 1 : -1;

  if (dx_ != 0 || dy_ != 0) {
    m_ = std::abs(dy_) < std::abs(dx_) ? static_cast<float>(dy_) / dx_ : static_cast<float>(dx_) / dy_;
  }

  dx_ = std::abs(dx_);
  dy_ = std::abs(dy_);
}

std::optional<PVec2> PathGenerator::Next() {
  auto result = next_;
  if (next_) {
    update();
  }
  return result;
}

void PathGenerator::update() {
  if (*next_ == end_) {
    next_ = {};
    return;
  }

  if (dy_ < dx_) {
    // Advance by x.
    next_->x += sx;
    if (next_->x == end_.x) {
      next_->y = end_.y;
    }
    else {
      auto y         = static_cast<float>(start_.y) + m_ * static_cast<float>(next_->x - start_.x);
      auto y_rounded = std::lroundl(y);
      next_->y       = y_rounded;
    }
  }
  else {
    // Advance by y.
    next_->y += sy;
    if (next_->y == end_.y) {
      next_->x = end_.x;
    }
    else {
      auto x         = static_cast<float>(start_.x) + m_ * static_cast<float>(next_->y - start_.y);
      auto x_rounded = std::lroundl(x);
      next_->x       = x_rounded;
    }
  }
}

}  // namespace pixelengine
