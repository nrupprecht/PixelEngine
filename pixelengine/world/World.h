//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <array>
#include <vector>

#include <Lightning/Lightning.h>

#include "pixelengine/graphics/Color.h"
#include "pixelengine/node/Node.h"
#include "pixelengine/utility/Utility.h"
#include "pixelengine/utility/Vec2.h"
#include "pixelengine/world/BoundingBox.h"

namespace pixelengine::world {

// Forward declare.
class World;


//! \brief The phase of matter of a material.
enum class PhaseOfMatter : uint8_t {
  SOLID  = 0x1,
  LIQUID = 0x2,
  GAS    = 0x4,
  POWDER = 0x8,
};

//! \brief The physical properties of a material
struct Material {
  //! \brief The mass of the material.
  float mass = 1.0;

  float friction = 0.5;

  //! \brief "Terminal velocity," in squares per second.
  float max_speed = 250.0;

  //! \brief Whether or not the material is fixed in place.
  bool is_rigid = false;

  //! \brief The phase of matter of the material.
  PhaseOfMatter phase_of_matter = PhaseOfMatter::SOLID;

  // ===========================================================================
  //  Convenience functions.
  // ===========================================================================

  [[nodiscard]] bool IsSolid() const noexcept {
    return static_cast<uint8_t>(phase_of_matter) & static_cast<uint8_t>(PhaseOfMatter::SOLID);
  }

  [[nodiscard]] bool IsLiquid() const noexcept {
    return static_cast<uint8_t>(phase_of_matter) & static_cast<uint8_t>(PhaseOfMatter::LIQUID);
  }

  [[nodiscard]] bool IsGas() const noexcept {
    return static_cast<uint8_t>(phase_of_matter) & static_cast<uint8_t>(PhaseOfMatter::GAS);
  }

  [[nodiscard]] bool IsPowder() const noexcept {
    return static_cast<uint8_t>(phase_of_matter) & static_cast<uint8_t>(PhaseOfMatter::POWDER);
  }

  [[nodiscard]] bool IsSolidOrPowder() const noexcept { return IsSolid() || IsPowder(); }

  [[nodiscard]] bool IsLiquidOrGas() const noexcept { return IsLiquid() || IsGas(); }
};

constexpr Material AIR {.phase_of_matter = PhaseOfMatter::GAS};
constexpr Material SAND {.mass = 2.0, .is_rigid = false, .phase_of_matter = PhaseOfMatter::POWDER};
constexpr Material WATER {.mass = 1.5, .is_rigid = false, .phase_of_matter = PhaseOfMatter::LIQUID};
constexpr Material DIRT {.mass = 3.0, .is_rigid = true, .phase_of_matter = PhaseOfMatter::SOLID};


//! \brief Class that represents how a square "behaves," i.e., its physical properties.
class SquareBehavior {
public:
  virtual ~SquareBehavior() = default;

  //! \brief Update the square within the world.
  //!
  //! \return Returns a bounding box around all the locations the square moved to during the update.
  //!         The bounding box is an empty bounding box if the square was completely blocked.
  virtual BoundingBox Update(float dt, long long x, long long y, World& world) const = 0;

  //! \brief Notify square that it has been "bumped" or "rubbed" against by another square.
  virtual void _onBump([[maybe_unused]] class Square& this_square,
                       [[maybe_unused]] class Square& other) const {}
};

//! \brief Represents a single square of material.
class Square {
public:
  Square() = default;

  Square(bool occupied, Color color, const Material* material, const SquareBehavior* behavior)
      : is_occupied(occupied)
      , color(color)
      , material(material)
      , behavior(behavior) {}

  //! \brief Whether the square is occupied.
  bool is_occupied = false;

  //! \brief Set to true when a square is successfully "bumped" by another square, or when the square has
  //!        non-zero velocity, or when the square otherwise needs an update (e.g., upon initialization).
  bool is_active = true;

  //! \brief Is the square currently in free fall?
  bool is_free_falling = true;

  //! \brief Used to determine the base color of the square.
  Color color;

  //! \brief The material the square is made out of.
  const Material* material = &AIR;

  const SquareBehavior* behavior {};

  //! \brief The velocity, in squares per second.
  Vec2 velocity {};
  //! \brief The "remainder" velocity that was "unused" last update.
  Vec2 remainder {};

  //! \brief How many times the square was moved during the last update.
  unsigned num_moves = 0;

  void UpdateKinematics(float dt, const World& world);

  void IncreaseMoves() { ++num_moves; }

  void DecreaseMoves() { num_moves = 0 < num_moves ? num_moves - 1 : 0; }
};

//! brief The world interface. Allows for accessing pixels / squares, but doesn't put any requirements on
//!       how the world is stored, cached, saved, updated, etc.
class World : public Node {
public:
  [[nodiscard]] const Square& GetSquare(long long x, long long y) const { return getSquare(x, y); }
  [[nodiscard]] Square& GetSquare(long long x, long long y) { return getSquare(x, y); }
  [[nodiscard]] const Square& GetSquare(PVec2 vec) const { return getSquare(vec.x, vec.y); }
  [[nodiscard]] Square& GetSquare(PVec2 vec) { return getSquare(vec.x, vec.y); }
  void SetSquare(long long x, long long y, const Square& square) { setSquare(x, y, square); }

  [[nodiscard]] bool IsValidSquare(long long x, long long y) const { return isValidSquare(x, y); }
  [[nodiscard]] bool IsValidSquare(PVec2 vec) const { return isValidSquare(vec.x, vec.y); }

  [[nodiscard]] virtual float GetGravity() const = 0;

private:
  [[nodiscard]] virtual const Square& getSquare(long long x, long long y) const = 0;
  [[nodiscard]] virtual Square& getSquare(long long x, long long y)             = 0;
  virtual void setSquare(long long x, long long y, const Square& square)        = 0;
  [[nodiscard]] virtual bool isValidSquare(long long x, long long y) const      = 0;

  // ===========================================================================
  //  Node overrides.
  // ===========================================================================

  [[nodiscard]] World* _setWorld(World*) final { return this; }
};


//! \brief Stationary square behavior. The square will never move.
class Stationary : public SquareBehavior {
public:
  BoundingBox Update([[maybe_unused]] float dt,
                     [[maybe_unused]] long long x,
                     [[maybe_unused]] long long y,
                     [[maybe_unused]] World& world) const override {
    return {};
  }
};

// class PowderBehavior : public SquareBehavior {
// public:
//   BoundingBox Update(float dt, long long x, long long y, World& world) const override;
//
// private:
//   bool interact(Square& current_square,
//                 PVec2 current_position,
//                 Square& other_square,
//                 PVec2 other_position,
//                 World& world) const;
// };

//! \brief Physics square behavior.
class Physics : public SquareBehavior {
public:
  constexpr explicit Physics(bool allow_sideways) : allow_sideways_(allow_sideways) {}

  BoundingBox Update(float dt, long long x, long long y, World& world) const override {
    auto& square = world.GetSquare(x, y);

    [[maybe_unused]] std::size_t iterations = 0;
    bool is_blocked                         = false;
    auto v                                  = std::fabsf(square.velocity.y * dt);

    BoundingBox bounding_box;

    auto original_x = x, original_y = y;
    bool did_update = false;
    for (; 1.f <= v; ++iterations) {
      std::tie(x, y, is_blocked) = singleUpdate(x, y, v, world);
      if (!is_blocked) {
        bounding_box.Update(x, y);
        did_update = true;
      }
    }

    // Random chance for one more update. We always do the update, so we can check if the block is blocked.
    int new_x = x, new_y = y;
    std::tie(new_x, new_y, is_blocked) = singleUpdate(x, y, v, world);
    if (!is_blocked && randf() < v - std::floorf(v)) {
      bounding_box.Update(x, y);
      ++iterations;
      did_update = true;
    }
    else {
      // Undo the movement.
      using std::swap;
      swap(world.GetSquare(x, y), world.GetSquare(new_x, new_y));

      // Undo the move count.
      world.GetSquare(x, y).DecreaseMoves();
      world.GetSquare(new_x, new_y).DecreaseMoves();
    }

    if (did_update) {
      // Mark the original square as having been updated (since whatever square the original square moved to
      // was swapped to this square).
      bounding_box.Update(original_x, original_y);
    }

    if (!is_blocked) {
      // If the square didn't move at all, it might because it is moving very slowly.
      bounding_box.Update(x, y);
    }

    return bounding_box;
  }

private:
  std::tuple<long long, long long, bool> singleUpdate(long long x,
                                                      long long y,
                                                      float& v,
                                                      World& world) const {
    v -= 1.f;

    auto& square = world.GetSquare(x, y);

    if (trySwap(world, square, x, y, 0, -1)) {
      return {x, y - 1, false};
    }

    if (randf() < 0.5) {
      if (trySwap(world, square, x, y, -1, -1)) {
        return {x - 1, y - 1, false};
      }
      if (trySwap(world, square, x, y, 1, -1)) {
        return {x + 1, y - 1, false};
      }
    }
    else {
      if (trySwap(world, square, x, y, 1, -1)) {
        return {x + 1, y - 1, false};
      }
      if (trySwap(world, square, x, y, -1, -1)) {
        return {x - 1, y - 1, false};
      }
    }

    if (allow_sideways_) {
      if (randf() < 0.5) {
        if (trySwap(world, square, x, y, -1, 0)) {
          return {x - 1, y, false};
        }
        if (trySwap(world, square, x, y, 1, 0)) {
          return {x + 1, y, false};
        }
      }
      else {
        if (trySwap(world, square, x, y, 1, 0)) {
          return {x + 1, y, false};
        }
        if (trySwap(world, square, x, y, -1, 0)) {
          return {x - 1, y, false};
        }
      }
    }
    return {x, y, true};
  }

  static bool trySwap(World& world, Square& square, long long x, long long y, int dx, int dy) {
    if (!world.IsValidSquare(x + dx, y + dy)) {
      return false;
    }

    auto& candidate = world.GetSquare(x + dx, y + dy);
    auto& material  = square.material;
    if (!candidate.material->is_rigid && candidate.material->mass < material->mass) {
      std::swap(square, candidate);

      square.IncreaseMoves();
      candidate.IncreaseMoves();

      // TODO: Experience some drag from the collision. The heavier the material, the more the drag.
      //  Also depends on a viscocity?

      return true;
    }
    return false;
  }

  bool allow_sideways_;
};


class FallingPhysics : public Physics {
public:
  constexpr FallingPhysics() : Physics(false) {}
};

class LiquidPhysics : public Physics {
public:
  constexpr LiquidPhysics() : Physics(true) {}
};


// ====

class PowderPhysics : public SquareBehavior {
public:
  constexpr PowderPhysics() = default;

  BoundingBox Update(float dt, long long x, long long y, World& world) const override {
    auto& square = world.GetSquare(x, y);

    bool is_blocked = false;
    auto vy         = std::fabsf(square.velocity.y * dt + square.remainder.y);
    auto vx         = square.velocity.x * dt + square.remainder.x;

    LOG_SEV(Info) << "Vx, Vy = (" << vx << ", " << vy << "), Vec = " << square.velocity
                  << ", Rem = " << square.remainder;

    BoundingBox bounding_box;

    auto original_x = x, original_y = y;
    bool did_update        = false;
    std::size_t iterations = 0;
    for (; 1.f <= vy || (!world.GetSquare(x, y).is_free_falling && 1.f <= std::abs(vx)); ++iterations) {
      std::tie(x, y, is_blocked) = singleUpdate(x, y, vx, vy, dt, world);
      if (!is_blocked) {
        bounding_box.Update(x, y);
        did_update = true;
      }
    }
    // Put the remaining "moves" into the remainder.
    auto& square_in_new_place       = world.GetSquare(x, y);
    square_in_new_place.remainder.x = vx;
    square_in_new_place.remainder.y = -vy;

    if (did_update) {
      // Mark the original square as having been updated (since whatever square the original square moved to
      // was swapped to this square).
      bounding_box.Update(original_x, original_y);
    }

    if (!is_blocked) {
      // If the square didn't move at all, it might because it is moving very slowly.
      bounding_box.Update(x, y);
    }

    return bounding_box;
  }

private:
  std::tuple<long long, long long, bool> singleUpdate(
      long long x, long long y, float& vx, float& vy, float dt, World& world) const {
    auto& square = world.GetSquare(x, y);

    auto apply_friction = [&](long dx, long dy) {
      auto& square_in_new_place = world.GetSquare(x + dx, y + dy);
      // Friction.
      constexpr auto reduction = 0.85f;
      vx *= reduction;
      square_in_new_place.velocity.x *= reduction;
      if (std::abs(square_in_new_place.velocity.x) < 1.) {
        square_in_new_place.velocity.x = 0.f;
        vx                             = 0.f;
      }
    };

    // If free falling, only fall, even if there is x velocity.

    if (trySwap(world, square, x, y, 0, -1)) {
      vy = std::max(vy - 1.f, 0.f);
      // The square is now "free falling" if it was not before.
      world.GetSquare(x, y - 1).is_free_falling = true;
      return {x, y - 1, false};
    }

    // Could not fall.
    if (square.is_free_falling) {
      // Turn some of the velocity into horizontal velocity.
      auto additional_vx = 0.5f * square.velocity.y * (randf() - 0.5f);
      square.velocity.x += additional_vx;
      vx += additional_vx * dt;
      vx                     = vx < 0 ? std::min(-1.f, vx) : std::max(1.f, vx);
      square.is_free_falling = false;
    }
    // Set y velocity to 0, since it hit something.
    vy                 = 0.f;
    square.velocity.y  = 0.f;
    square.remainder.y = 0.f;

    if (std::abs(vx) < 1.) {
      // Not blocked, just not moving fast enough to move again this turn.
      return {x, y, false};
    }

    // Try diagonal.
    if (vx < 0.) {
      if (trySwap(world, square, x, y, -1, -1)) {
        vx += 1.f;
        apply_friction(-1, -1);
        return {x - 1, y - 1, false};
      }
      if (trySwap(world, square, x, y, 1, -1)) {
        vx *= -1;
        square.velocity.x *= -1;
        vx -= 1.f;
        apply_friction(1, -1);
        return {x - 1, y + 1, false};
      }
    }
    else {
      if (trySwap(world, square, x, y, 1, -1)) {
        vx -= 1.f;
        apply_friction(1, -1);
        return {x + 1, y - 1, false};
      }
      if (trySwap(world, square, x, y, -1, -1)) {
        vx *= -1;
        square.velocity.x *= -1;
        vx += 1.f;
        apply_friction(-1, -1);
        return {x - 1, y - 1, false};
      }
    }

    // Could not fall diagonally. Slide. This causes friction.

    if (vx < 0.) {
      if (trySwap(world, square, x, y, -1, 0)) {
        vx = std::min(0.f, vx + 1.f);
        apply_friction(-1, 0);

        return {x - 1, y, false};
      }
    }
    else {
      if (trySwap(world, square, x, y, 1, 0)) {
        vx = std::max(0.f, vx - 1.f);
        apply_friction(1, 0);

        return {x + 1, y, false};
      }
    }

    // Totally blocked.
    vx                 = 0.f;
    square.velocity.x  = 0.f;
    square.remainder.x = 0.f;

    return {x, y, true};
  }

  static bool trySwap(World& world, Square& square, long long x, long long y, int dx, int dy) {
    if (!world.IsValidSquare(x + dx, y + dy)) {
      return false;
    }

    auto& candidate = world.GetSquare(x + dx, y + dy);
    auto& material  = square.material;
    if (!candidate.material->is_rigid && candidate.material->mass < material->mass) {
      std::swap(square, candidate);

      square.IncreaseMoves();
      candidate.IncreaseMoves();

      // TODO: Experience some drag from the collision. The heavier the material, the more the drag.
      //  Also depends on a viscocity?

      return true;
    }
    return false;
  }
};

}  // namespace pixelengine::world