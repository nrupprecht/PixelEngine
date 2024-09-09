//
// Created by Nathaniel Rupprecht on 9/9/24.
//

#include "pixelengine/world/World.h"
// Other files.

namespace pixelengine::world {

void World::evolve(float raw_dt) {

  auto dt = std::min(1.f / 30.f, raw_dt);

  auto begin_evolution_time = std::chrono::high_resolution_clock::now();

  // Reset was-moved flags.
  for (auto y = 0; y < chunk_height_; ++y) {
    for (auto x = 0; x < chunk_width_; ++x) {
      auto&& square    = getSquare(x, y);
      square.was_moved = false;
    }
  }

  // Get the region in which updates need to occur.
  active_region_.Expand(1);
  auto [x_min, x_max, y_min, y_max] =
      active_region_.Clip(static_cast<long long>(chunk_width_), static_cast<long long>(chunk_height_));

  BoundingBox bounding_box;

  auto update = [this, dt, &bounding_box](long long x, long long y) {
    auto&& square = getSquare(x, y);
    if (!square.is_occupied || square.material->is_rigid || !square.behavior || square.was_moved) {
      return;
    }

    square.UpdateKinematics(dt, gravity_);
    if (auto bb = square.behavior->Update(dt, x, y, *this); !bb.IsEmpty()) {
      bounding_box.Update(bb);
      square.was_moved = true;
    }
  };

  // Update motion.
  for (auto y = y_min; y <= y_max; ++y) {
    if (randf() < 0.5) {
      for (auto x = x_min; x <= x_max; ++x) {
        update(x, y);
      }
    }
    else {
      for (auto x = x_max; x >= x_min; --x) {
        update(x, y);
      }
    }
  }

  // Update active region.
  active_region_ = bounding_box;

  // TODO: Other updates, e.g. temperature, objects catching fire, reacting, etc.

  auto end_evoluation_time = std::chrono::high_resolution_clock::now();

  // Get the time it took to evolve.
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_evoluation_time - begin_evolution_time);

  auto evolution_time = static_cast<float>(duration.count()) / 1'000'000.f;
  std::cout << evolution_time << std::endl;
}

}  // namespace pixelengine::world