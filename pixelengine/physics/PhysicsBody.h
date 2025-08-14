#pragma once

#include "pixelengine/node/Node.h"
#include "pixelengine/world/World.h"

namespace pixelengine::physics {

class InteractionSystem;

using pixelengine::Vec2;

struct BodyState {
  bool blocked_left {}, blocked_right {}, blocked_up {}, blocked_down {};

  bool blocked_bottom_left {}, blocked_bottom_right {}, blocked_top_left {}, blocked_top_right {};

  [[nodiscard]] bool AnyBlocked() const noexcept {
    return blocked_left || blocked_right || blocked_up || blocked_down || blocked_bottom_left
        || blocked_bottom_right || blocked_top_left || blocked_top_right;
  }

  [[nodiscard]] bool AnyPrincipleBlocked() const noexcept {
    return blocked_left || blocked_right || blocked_up || blocked_down;
  }

  friend std::ostream& operator<<(std::ostream& out, const BodyState& body) {
    if (!body.AnyBlocked()) {
      out << "unblocked";
    }
    else {
      // clang-format off
      out << "blocked:";
      // Principle directions
      if (body.blocked_left) out << "L";
      if (body.blocked_right) out << "R";
      if (body.blocked_down) out << "D";
      if (body.blocked_up) out << "U";
      // Minor directions
      if (body.blocked_top_left) out << "*";
      if (body.blocked_top_right) out << "^";
      if (body.blocked_bottom_left) out << "+";
      if (body.blocked_bottom_right) out << "v";
      // clang-format on
    }
    return out;
  }
};

class PhysicsBody : public Node {
  friend class InteractionSystem;

public:
  PhysicsBody(PVec2 position, unsigned width, unsigned height, Vec2 velocity = {});

  [[nodiscard]] Vec2 GetVelocity() const { return velocity_; }

  [[nodiscard]] const BodyState& GetState() const noexcept { return state_; }

  [[nodiscard]] PVec2 GetPosition() const noexcept { return position_; }

protected:
  void clearVelocity();

  void setVelocityX(float vx) { velocity_.x = vx; }
  void setVelocityY(float vy) { velocity_.y = vy; }

  void addVelocityX(float dvx);
  void addVelocityY(float dvy);

  void applyForceX(float fx);
  void applyForceY(float fy);

  virtual void _onAddedToInteractionSystem([[maybe_unused]] InteractionSystem* interaction_system) {}
  virtual void _onRemovedFromInteractionSystem([[maybe_unused]] InteractionSystem* interaction_system) {}

private:
  void _interactWithWorld(world::World* world) override;
  void _updatePhysics(float dt, const world::World* world) override;

  [[nodiscard]] Vec2 _additionalOffset() const override { return position_.To<float>(); }

  void updateBodyPhysics(float dt, const world::World* world);
  void moveBody(world::World& world);

  bool moveX(bool right, world::World& world);
  bool moveY(bool up, world::World& world);

  [[nodiscard]] bool isBlockedDown(world::World& world) const;

  //! \brief Returns the height of the step to the left or right, in pixels.
  [[nodiscard]] unsigned heightOfStep(bool move_right, world::World& world) const;

  bool tryStep(bool move_right, world::World& world);

  //! The position (bottom left corner) of the physics body.
  PVec2 position_ {};
  Vec2 velocity_ {};
  Vec2 force_ {};
  Vec2 remainder_ {};

  //! \brief The body width and height in pixels.
  //!
  //! \note assumes the body is an axis aligned box.
  unsigned width_ {}, height_ {};

  bool can_step_ {true};
  unsigned stepping_height_ = 4;

  float mass_ = 1.;

  BodyState state_ {}, last_state_ {};
};

}  // namespace pixelengine::physics