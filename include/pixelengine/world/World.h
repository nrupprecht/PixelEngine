//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <array>
#include <vector>

#include <Lightning/Lightning.h>

#include "pixelengine/Node.h"
#include "pixelengine/graphics/Color.h"
#include "pixelengine/utility/Utility.h"
#include "pixelengine/world/BoundingBox.h"

namespace pixelengine::world {


//! \brief The phase of matter of a material.
enum class PhaseOfMatter : uint8_t {
  SOLID  = 0x1,
  LIQUID = 0x2,
  GAS    = 0x4
};

//! \brief The physical properties of a material
struct Material {
  //! \brief The mass of the material.
  float mass = 1.0;

  //! \brief "Terminal velocity," in squares per second.
  float max_speed = 250.0;

  //! \brief Whether or not the material is fixed in place.
  bool is_rigid = false;
};

constexpr Material AIR {};
constexpr Material SAND {.mass = 2.0, .is_rigid = false};
constexpr Material WATER {.mass = 1.5, .is_rigid = false};
constexpr Material DIRT {.mass = 3.0, .is_rigid = true};


//! \brief Class that represents how a square "behaves," i.e., its physical properties.
class SquareBehavior {
public:
  virtual ~SquareBehavior() = default;

  //! \brief Update the square within the world.
  //!
  //! \return Returns a bounding box around all the locations the square moved to during the update.
  //!         The bounding box is an empty bounding box if the square was completely blocked.
  virtual BoundingBox Update(float dt, std::size_t x, std::size_t y, class World& world) const = 0;
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

  //! \brief Flag that lets the engine track whether a square has already been handled.
  // bool already_updated = false;

  //! \brief Whether the square is occupied.
  bool is_occupied = false;

  //! \brief Used to determine the base color of the square.
  Color color;

  //! \brief The material the square is made out of.
  const Material* material = &AIR;

  const SquareBehavior* behavior {};

  //! \brief The velocity in the y direction, in squares per second.
  float velocity_y = 0.f;

  //! \brief For later - the temperature, in kelvin.
  // float temperature = 300.f;

  //! \brief How many times the square was moved during the last update.
  unsigned num_moves = 0;

  void UpdateKinematics(float dt, float gravity) {
    velocity_y += gravity * dt;
    velocity_y = std::max(-material->max_speed, std::min(material->max_speed, velocity_y));
  }

  void IncreaseMoves() { ++num_moves; }

  void DecreaseMoves() { num_moves = 0 < num_moves ? num_moves - 1 : 0; }
};

//! brief The world interface. Allows for accessing pixels / squares, but doesn't put any requirements on
//!       how the world is stored, cached, saved, updated, etc.
class World : public Node {
public:
  [[nodiscard]] const Square& GetSquare(long long x, long long y) const { return getSquare(x, y); }
  [[nodiscard]] Square& GetSquare(long long x, long long y) { return getSquare(x, y); }
  void SetSquare(long long x, long long y, const Square& square) { setSquare(x, y, square); }

  [[nodiscard]] bool IsValidSquare(long long x, long long y) const { return isValidSquare(x, y); }

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
  BoundingBox Update(float dt, std::size_t x, std::size_t y, World& world) const override { return {}; }
};

//! \brief Physics square behavior.
class Physics : public SquareBehavior {
public:
  constexpr explicit Physics(bool allow_sideways) : allow_sideways_(allow_sideways) {}

  BoundingBox Update(float dt, std::size_t x, std::size_t y, World& world) const override {
    auto& square = world.GetSquare(x, y);

    std::size_t iterations = 0;
    bool is_blocked        = false;
    auto v                 = std::fabsf(square.velocity_y * dt);

    BoundingBox bounding_box;

    for (; 1.f <= v; ++iterations) {
      std::tie(x, y, is_blocked) = singleUpdate(x, y, v, world);
      if (!is_blocked) {
        bounding_box.Update(x, y);
      }
    }

    // Random chance for one more update. We always do the update, so we can check if the block is blocked.
    int new_x = x, new_y = y;
    std::tie(new_x, new_y, is_blocked) = singleUpdate(x, y, v, world);
    if (!is_blocked && randf() < v - std::floorf(v)) {
      bounding_box.Update(x, y);
      ++iterations;
    }
    else {
      // Undo the movement.
      using std::swap;
      swap(world.GetSquare(x, y), world.GetSquare(new_x, new_y));

      // Undo the move count.
      world.GetSquare(x, y).DecreaseMoves();
      world.GetSquare(new_x, new_y).DecreaseMoves();
    }

    if (!is_blocked) {
      // If the square didn't move at all, it might because it is moving very slowly.
      bounding_box.Update(x, y);
    }

    return bounding_box;
  }

private:
  std::tuple<std::size_t, std::size_t, bool> singleUpdate(std::size_t x,
                                                          std::size_t y,
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

  static bool trySwap(World& world, Square& square, std::size_t x, std::size_t y, int dx, int dy) {
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


}  // namespace pixelengine::world