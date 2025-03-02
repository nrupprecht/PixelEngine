//
// Created by Nathaniel Rupprecht on 10/8/24.
//

#include "pixelengine/world/World.h"
// Other files.
#include "pixelengine/utility/PathGenerator.h"

namespace pixelengine::world {

void Square::UpdateKinematics(float dt, const World& world) {
  if (material->is_rigid) {
    return;
  }
  if (/*is_free_falling && is_active && */ material->IsPowder() || material->IsLiquid()) {
    velocity.y += world.GetGravity() * dt;

    velocity.y = std::max(-material->max_speed, std::min(material->max_speed, velocity.y));
    velocity.x = std::max(-material->max_speed, std::min(material->max_speed, velocity.x));
  }
}

bool attemptSwap(Square& square, long long x1, long long y1, World& world) {
  if (!world.IsValidSquare(x1, y1)) {
    return false;
  }
  auto& candidate = world.GetSquare(x1, y1);
  if (!candidate.is_occupied) {
    using std::swap;
    swap(square, candidate);
    return true;
  }
  return false;
}

// BoundingBox PowderBehavior::Update(float dt, long long x, long long y, World& world) const {
//   auto& square = world.GetSquare(x, y);
//
//   BoundingBox bounding_box;
//
//   // Generate powder path.
//   PVec2 position {x, y};
//   square.velocity += square.remainder;
//
//   if (square.is_active) {
//   }
//
//
//   auto [end_point, remainder] = AddWithRemainder(position, square.velocity);
//   PathGenerator path_generator(position, end_point);
//
//   // First call will return the start point.
//   auto last_point = *path_generator.Next();
//
//   while (auto opt_next_point = path_generator.Next()) {
//     auto next_point      = *opt_next_point;
//     auto& current_square = world.GetSquare(last_point);
//
//     // Interact with the next point.
//     bool was_blocked = interact(current_square, last_point, world.GetSquare(next_point), next_point, world);
//     if (was_blocked) {
//       break;
//     }
//
//     last_point = {x, y};
//   }
//
//
//   return bounding_box;
// }

// bool PowderBehavior::interact(Square& current_square,
//                               PVec2 current_position,
//                               Square& other_square,
//                               PVec2 other_position,
//                               World& world) const {
//   if (!other_square.is_occupied || other_square.material->IsLiquidOrGas()) {
//     // Bump neighbors.
//     if (auto pos = current_position - PVec2{1, 0}; world.IsValidSquare(pos)) {
//       auto& left_square = world.GetSquare(pos);
//       left_square.behavior->_onBump(left_square, current_square);
//     }
//     if (auto pos = current_position + PVec2{1, 0}; world.IsValidSquare(pos)) {
//       auto& right_square = world.GetSquare(pos);
//       right_square.behavior->_onBump(right_square, current_square);
//     }
//
//     if (other_position.y < current_position.y) {
//       current_square.is_free_falling = true;
//     }
//
//     // Move.
//     std::swap(current_square, other_square);
//   }
//   else {
//     // Is a solid or powder.
//     if (current_square.is_free_falling) {
//       // Convert the velocity into some x velocity.
//
//
//     }
//     return true;
//   }
//
// }

}  // namespace pixelengine::world
