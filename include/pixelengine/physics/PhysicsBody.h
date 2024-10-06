#pragma once

#include "pixelengine/Node.h"
#include "pixelengine/world/World.h"

namespace pixelengine::physics {

using pixelengine::Vec2;

struct BodyState {
  bool blocked_left {}, blocked_right {}, blocked_up {}, blocked_down {};

  bool blocked_bottom_left {}, blocked_bottom_right {}, blocked_top_left {}, blocked_top_right {};

  // TODO: State transition events.
};

class PhysicsBody : public Node {
public:
  PhysicsBody(Vec2 position, float width, float height, Vec2 velocity = {});

  [[nodiscard]] Vec2 GetPosition() const { return position_; }
  [[nodiscard]] Vec2 GetVelocity() const { return velocity_; }

protected:
  void clearVelocity();
  void addVelocityX(float dvx);
  void addVelocityY(float dvy);

private:
  void _interactWithWorld(world::World* world) override;
  void _updatePhysics(float dt) override;

  void updateBodyPhysics(float dt);
  void checkCollisions(world::World& world);

  Vec2 position_ {};
  Vec2 velocity_ {};
  float width_ {}, height_ {};

  float mass_ = 1.;

  BodyState state_ {}, last_state_ {};
};

}  // namespace pixelengine::physics