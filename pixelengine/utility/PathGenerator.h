//
// Created by Nathaniel Rupprecht on 10/6/24.
//

#pragma once

#include "pixelengine/utility/Vec2.h"

namespace pixelengine {

//! \brief Generate a path from start to end.
//!
//! Can act like a generator, returning the next point in the path.
class PathGenerator {
public:
  //! \brief Construct a generator that will create a path from start to end.
  PathGenerator(PVec2 start, PVec2 end);

  //! \brief Get the next point in the path, or nullopt if there are no further points.
  std::optional<PVec2> Next();

private:
  void update();

  PVec2 start_, end_;
  std::optional<PVec2> next_;
  long dx_, dy_;
  float m_ = 0.f;
  int sx, sy;
};

}  // namespace pixelengine