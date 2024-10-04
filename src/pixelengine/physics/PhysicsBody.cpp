//
// Created by Nathaniel Rupprecht on 9/28/24.
//

#include "pixelengine/physics/PhysicsBody.h"

namespace pixelengine::physics {

PhysicsBody::PhysicsBody(Vec2 position, float width, float height, Vec2 velocity)
    : position_(position)
    , width_(width)
    , height_(height)
    , velocity_(velocity) {}

void PhysicsBody::_interactWithWorld(world::World& world) {
  Node::_interactWithWorld(world);
  checkCollisions(world);
}

void PhysicsBody::_updatePhysics(float dt) {
  Node::_updatePhysics(dt);
  updateBodyPhysics(dt);
}

void PhysicsBody::updateBodyPhysics(float dt) {
  // Update the position of the body based on the velocity and the time step.

  // TODO: Get gravity from somewhere.... like the world.
  velocity_.y += -9.8f * dt;

  if (state_.blocked_down) {
    velocity_.y = std::max(0.f, velocity_.y);
  }
  if (state_.blocked_up) {
    velocity_.y = std::min(0.f, velocity_.y);
  }
  if (state_.blocked_left) {
    velocity_.x = std::max(0.f, velocity_.x);
  }
  if (state_.blocked_right) {
    velocity_.x = std::min(0.f, velocity_.x);
  }

  position_.x += velocity_.x * dt;
  position_.y += velocity_.y * dt;
}

void PhysicsBody::checkCollisions(world::World& world) {
  // Check around the border of the rectangle for collisions with the world.

  // Find the pixels that surround the body.
  auto bottom = static_cast<long long>(std::floor(position_.y));
  auto top    = static_cast<long long>(std::ceil(position_.y + height_));
  auto left   = static_cast<long long>(std::floor(position_.x));
  auto right  = static_cast<long long>(std::ceil(position_.x + width_));

  BodyState new_state;

  // Check left side.
  for (auto y = bottom; y < top; ++y) {
    auto&& square = world.GetSquare(left - 1, y);
    if (square.is_occupied) {
      new_state.blocked_left = true;
      break;
    }
  }

  // Check right side.
  for (auto y = bottom; y < top; ++y) {
    auto&& square = world.GetSquare(right, y);
    if (square.is_occupied) {
      new_state.blocked_right = true;
      break;
    }
  }

  // Check top side.
  for (auto x = left; x < right; ++x) {
    auto&& square = world.GetSquare(x, top);
    if (square.is_occupied) {
      new_state.blocked_up = true;
      break;
    }
  }

  // Check bottom side.
  for (auto x = left; x < right; ++x) {
    auto&& square = world.GetSquare(x, bottom - 1);
    if (square.is_occupied) {
      new_state.blocked_down = true;
      break;
    }
  }

  // ========================
  // Check corners
  // ========================

  auto check_corner = [&](long long x, long long y) {
    auto&& square = world.GetSquare(x, y);
    return square.is_occupied;
  };

  // Top left corner
  check_corner(left - 1, top + 1);
  // Top right corner
  check_corner(right + 1, top + 1);
  // Bottom left corner
  check_corner(left - 1, bottom - 1);
  // Bottom right corner
  check_corner(right + 1, bottom - 1);

  // TODO: Events when state changes?

  last_state_ = state_;
  state_      = new_state;
}

}  // namespace pixelengine::physics