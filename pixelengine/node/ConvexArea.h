#pragma once

#include "pixelengine/node/Area.h"

namespace pixelengine::node {

class ConvexArea : public Area {
public:
  ConvexArea(std::span<Vec2> vertices) : vertices_(vertices.begin(), vertices.end()) {
    PIXEL_ASSERT(vertices_.size() >= 3, "ConvexArea must have at least 3 vertices.");
  }

  ConvexArea(std::vector<Vec2> vertices) : vertices_(std::move(vertices)) {
    PIXEL_ASSERT(vertices_.size() >= 3, "ConvexArea must have at least 3 vertices.");
  }

private:
  //! \brief Check whether a point lies within the convex body.
  bool pointWithinArea(const Vec2& point) const override {
    for (std::size_t i = 0; i < vertices_.size(); ++i) {
      std::size_t ip1 = (i + 1) % vertices_.size();

      Vec2 l                   = vertices_[ip1] - vertices_[i];
      Vec2 d                   = point - vertices_[i];
      const auto cross_product = (d ^ l);
      if (cross_product < 0.f) {
        return false;
      }
    }
    return true;
  }

  std::vector<Vec2> vertices_;
};

}  // namespace pixelengine::node