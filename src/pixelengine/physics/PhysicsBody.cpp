//
// Created by Nathaniel Rupprecht on 9/28/24.
//

#include "pixelengine/physics/PhysicsBody.h"
// Other files.
#include "pixelengine/utility/PathGenerator.h"

namespace pixelengine::physics {

PhysicsBody::PhysicsBody(PVec2 position, unsigned width, unsigned height, Vec2 velocity)
    : position_(position)
    , width_(width)
    , height_(height)
    , velocity_(velocity) {}

void PhysicsBody::clearVelocity() {
  velocity_ = {};
}

void PhysicsBody::addVelocityX(float dvx) {
  velocity_.x += dvx;
}

void PhysicsBody::addVelocityY(float dvy) {
  velocity_.y += dvy;
}

void PhysicsBody::applyForceX(float fx) {
  force_.x += fx;
}

void PhysicsBody::applyForceY(float fy) {
  force_.y += fy;
}

void PhysicsBody::_interactWithWorld(world::World* world) {
  Node::_interactWithWorld(world);
  if (world) {
    moveBody(*world);
  }
}

void PhysicsBody::_updatePhysics(float dt) {
  Node::_updatePhysics(dt);
  updateBodyPhysics(dt);
}

void PhysicsBody::updateBodyPhysics(float dt) {
  // Update the position of the body based on the velocity and the time step.

  // TODO: Get gravity from somewhere.... like the world.
  force_.y -= 25.f * mass_;

  velocity_.x += force_.x * dt / mass_;
  velocity_.y += force_.y * dt / mass_;

  // Limit velocity?
  velocity_.x = std::clamp(velocity_.x, -25.f, 25.f);
  velocity_.y = std::clamp(velocity_.y, -25.f, 25.f);

  force_ = {};  // Reset force.

  // TODO: More complex?
  // if (state_.blocked_down) {
  //   const float mu = 0.;
  //   if (velocity_.x < 0) {
  //     velocity_.x -= mu * mass_;
  //     velocity_.x = std::max(velocity_.x, 0.f);
  //   }
  //   else if (velocity_.x > 0) {
  //     velocity_.x += mu * mass_;
  //     velocity_.x = std::min(velocity_.x, 0.f);
  //   }
  //
  //   if (velocity_.x == 0) {
  //     remainder_.x = 0;
  //   }
  // }
}

void PhysicsBody::moveBody(world::World& world) {
  // Put the velocity into the remainder.
  remainder_ += velocity_;

  PVec2 end_position = position_;
  if (0 < remainder_.x) {
    end_position.x += std::lroundf(std::floor(remainder_.x));
    remainder_.x -= std::floor(remainder_.x);
  }
  else {
    end_position.x += std::lroundf(std::ceil(remainder_.x));
    remainder_.x -= std::ceil(remainder_.x);
  }
  // Y
  if (0 < remainder_.y) {
    end_position.y += std::lroundf(std::floor(remainder_.y));
    remainder_.y -= std::floor(remainder_.y);
  }
  else {
    end_position.y += std::lroundf(std::ceil(remainder_.y));
    remainder_.y -= std::ceil(remainder_.y);
  }


  PathGenerator generator(position_, end_position);
  bool blocked_x = false, blocked_y = false;
  while (auto next = generator.Next()) {
    if (!blocked_x && next->x != position_.x) {
      auto move_right = next->x > position_.x;
      if (!moveX(move_right, world)) {
        // If stepping is allowed, try to step.
        if (can_step_ && tryStep(move_right, world)) {
          continue;
        }
        // Else, or if stepping failed.
        blocked_x    = true;
        remainder_.x = 0.f;
        velocity_.x  = 0.f;
      }
    }
    if (!blocked_y && next->y != position_.y) {
      if (!moveY(next->y > position_.y, world)) {
        blocked_y    = true;
        remainder_.y = 0.f;
        velocity_.y  = 0.f;
      }
    }
  }

  // Check for collisions.

  // Find the pixels that surround the body.
  auto bottom = position_.y;
  auto top    = position_.y + height_;
  auto left   = position_.x;
  auto right  = position_.x + width_;

  BodyState new_state;

  // Check left side.
  for (auto y = bottom; y < top; ++y) {
    if (!world.IsValidSquare(left - 1, y)) {
      new_state.blocked_left = true;
      break;
    }
    auto&& square = world.GetSquare(left - 1, y);
    if (square.is_occupied && square.material->IsSolid()) {
      new_state.blocked_left = true;
      break;
    }
  }

  // Check right side.
  for (auto y = bottom; y < top; ++y) {
    if (!world.IsValidSquare(right + 1, y)) {
      new_state.blocked_right = true;
      break;
    }
    auto&& square = world.GetSquare(right + 1, y);
    if (square.is_occupied && square.material->IsSolid()) {
      new_state.blocked_right = true;
      break;
    }
  }

  // Check top side.
  for (auto x = left; x < right; ++x) {
    if (!world.IsValidSquare(x, top + 1)) {
      new_state.blocked_up = true;
      break;
    }
    auto&& square = world.GetSquare(x, top + 1);
    if (square.is_occupied && square.material->IsSolid()) {
      new_state.blocked_up = true;
      break;
    }
  }

  // Check bottom side.
  for (auto x = left; x < right; ++x) {
    if (!world.IsValidSquare(x, bottom - 1)) {
      new_state.blocked_down = true;
      break;
    }
    auto&& square = world.GetSquare(x, bottom - 1);
    if (square.is_occupied && square.material->IsSolid()) {
      new_state.blocked_down = true;
      break;
    }
  }

  // ========================
  // Check corners
  // ========================

  auto check_corner = [&](long long x, long long y) {
    if (!world.IsValidSquare(x, y)) {
      return true;
    }
    auto&& square = world.GetSquare(x, y);
    return square.is_occupied && square.material->IsSolid();
  };

  // Top left corner
  check_corner(left - 1, top + 1);
  // Top right corner
  check_corner(right + 1, top + 1);
  // Bottom left corner
  check_corner(left - 1, bottom - 1);
  // Bottom right corner
  check_corner(right + 1, bottom - 1);

  last_state_ = state_;
  state_      = new_state;
}

bool PhysicsBody::moveX(bool right, world::World& world) {
  long long column = right ? position_.x + width_ : position_.x - 1;

  for (auto y = position_.y; y < position_.y + height_; ++y) {
    if (!world.IsValidSquare(column, y)) {
      return false;
    }
    auto&& square = world.GetSquare(column, y);
    if (square.is_occupied && square.material->IsSolid()) {
      return false;
    }
  }

  position_.x += right ? 1 : -1;

  return true;
}

bool PhysicsBody::moveY(bool up, world::World& world) {
  long long row = up ? position_.y + height_ : position_.y - 1;

  for (auto x = position_.x; x < position_.x + width_; ++x) {
    if (!world.IsValidSquare(x, row)) {
      return false;
    }
    auto&& square = world.GetSquare(x, row);
    if (square.is_occupied && square.material->IsSolid()) {
      return false;
    }
  }

  position_.y += up ? 1 : -1;

  return true;
}

bool PhysicsBody::isBlockedDown(world::World& world) const {
  long long row = position_.y - 1;
  for (auto x = position_.x; x < position_.x + width_; ++x) {
    if (!world.IsValidSquare(x, row)) {
      return true;
    }
    auto&& square = world.GetSquare(x, row);
    if (square.is_occupied && square.material->IsSolid()) {
      return true;
    }
  }
  return false;
}

unsigned PhysicsBody::heightOfStep(bool right, world::World& world) const {
  long long column = right ? position_.x + width_ : position_.x - 1;

  for (auto y = position_.y + height_ - 1; 0 <= y; --y) {
    if (!world.IsValidSquare(column, y)) {
      return y - position_.y + 1;
    }
    auto&& square = world.GetSquare(column, y);
    if (square.is_occupied && square.material->IsSolid()) {
      return y - position_.y + 1;
    }
  }
  return 0;
}

bool PhysicsBody::tryStep(bool move_right, world::World& world) {
  // Stepping logic. Body must be on the ground to step.
  // Count the height of the highest blocker. If it is short enough, move up that number of times,
  // then move in the X direction.
  if (isBlockedDown(world)) {
    auto step_height = heightOfStep(move_right, world);
    if (step_height <= stepping_height_) {
      auto step = 0u;
      for (; step < step_height; ++step) {
        if (!moveY(true, world)) {
          return false;
        }
      }
      if (step == step_height && moveX(move_right, world)) {
        return true;
      }
    }
  }
  return false;
}

}  // namespace pixelengine::physics