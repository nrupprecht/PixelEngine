//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <array>
#include <vector>

#include <Lightning/Lightning.h>

#include "pixelengine/Color.h"
#include "pixelengine/Utility.h"

namespace pixelengine::world {

struct BoundingBox {
  BoundingBox() = default;
  BoundingBox(long long x, long long x2, long long y, long long y2)
      : x_min(x)
      , x_max(x2)
      , y_min(y)
      , y_max(y2) {}

  long long x_min = 0;
  long long x_max = -1;
  long long y_min = 0;
  long long y_max = -1;

  [[nodiscard]] bool IsEmpty() const { return x_max < x_min; }

  void Update(long long x, long long y) {
    if (x_max < x_min) {
      x_min = x_max = x;
      y_min = y_max = y;
    }
    else {
      x_min = std::min(x, x_min);
      x_max = std::max(x, x_max);
      y_min = std::min(y, y_min);
      y_max = std::max(y, y_max);
    }
  }

  void Update(const BoundingBox& other) {
    if (other.IsEmpty()) {
      return;
    }
    if (IsEmpty()) {
      *this = other;
      return;
    }
    x_min = std::min(x_min, other.x_min);
    x_max = std::max(x_max, other.x_max);
    y_min = std::min(y_min, other.y_min);
    y_max = std::max(y_max, other.y_max);
  }

  void Expand(long long amount) {
    if (x_min <= x_max) {
      x_min -= amount;
      x_max += amount;
      y_min -= amount;
      y_max += amount;
    }
  }

  std::array<long long, 4> Clip(long long width, long long height) {
    auto _x_min = std::max(0ll, x_min);
    auto _x_max = std::min(width - 1, x_max);
    auto _y_min = std::max(0ll, y_min);
    auto _y_max = std::min(height - 1, y_max);
    return {_x_min, _x_max, _y_min, _y_max};
  }
};


//! \brief The physical properties of a material
struct Material {
  //! \brief The mass of the material.
  float mass = 1.0;
  //! \brief "Terminal velocity," in squares per second.
  float max_speed = 500.f;

  //! \brief Whether or not the material is fixed in place.
  bool is_rigid = false;
};

constexpr Material AIR {};
constexpr Material SAND {2.0};
constexpr Material WATER {1.5};

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

  //! \brief Whether the square was moved during the last update.
  bool was_moved = false;

  void UpdateKinematics(float dt, float gravity) {
    velocity_y += gravity * dt;
    velocity_y = std::max(-material->max_speed, std::min(material->max_speed, velocity_y));
  }
};


class World {
public:
  World(std::size_t chunk_width, std::size_t chunk_height)
      : chunk_width_(chunk_width)
      , chunk_height_(chunk_height)
      , active_region_(0, static_cast<long long>(chunk_width), 0, static_cast<long long>(chunk_height))
      , squares_(chunk_width_ * chunk_height_) {}

  [[nodiscard]] const Square& GetSquare(std::size_t x, std::size_t y) const { return getSquare(x, y); }
  [[nodiscard]] Square& GetSquare(std::size_t x, std::size_t y) { return getSquare(x, y); }

  void SetSquare(std::size_t x, std::size_t y, const Square& square) {
    active_region_.Update(x, y);
    getSquare(x, y) = square;
  }

  void Update(float dt) { evolve(dt); }

  [[nodiscard]] std::size_t GetWidth() const { return chunk_width_; }
  [[nodiscard]] std::size_t GetHeight() const { return chunk_height_; }
  [[nodiscard]] float GetGravity() const { return gravity_; }

private:
  std::size_t chunk_width_;
  std::size_t chunk_height_;

  BoundingBox active_region_;

  //! \brief Acceleration due to gravity, in squares per second squared.
  float gravity_ = -20.;

  void evolve(float dt);

  Square& getSquare(std::size_t x, std::size_t y) {
    LL_ASSERT(x < chunk_width_ && y < chunk_height_, "out of bounds");
    return squares_[y * chunk_width_ + x];
  }

  [[nodiscard]] const Square& getSquare(std::size_t x, std::size_t y) const {
    LL_ASSERT(x < chunk_width_ && y < chunk_height_, "out of bounds");
    return squares_[y * chunk_width_ + x];
  }

  std::vector<Square> squares_;
};

//! \brief Stationary square behavior. The square will never move.
class Stationary : public SquareBehavior {
  BoundingBox Update(float dt, std::size_t x, std::size_t y, World& world) const override { return {}; }
};

//! \brief Physics square behavior.
class Physics : public SquareBehavior {
public:
  explicit Physics(bool allow_sideways) : allow_sideways_(allow_sideways) {}

  BoundingBox Update(float dt, std::size_t x, std::size_t y, World& world) const override {
    auto& square = world.GetSquare(x, y);

    std::size_t iterations = 0;
    bool is_blocked        = false;
    auto v                 = std::fabsf(square.velocity_y * dt);

    BoundingBox bounding_box;

    for (; 1.f <= v; ++iterations) {
      std::tie(x, y, is_blocked) = singleUpdate(x, y, v, world);
      if (!is_blocked) bounding_box.Update(x, y);
    }
    // Random chance for one more update.
    if (!is_blocked && randf() < v - std::floorf(v)) {
      std::tie(x, y, is_blocked) = singleUpdate(x, y, v, world);
      if (!is_blocked) bounding_box.Update(x, y);
      ++iterations;
    }

    if (iterations == 0) {
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
    if (dx < 0 && x == 0) {
      return false;
    }
    if (dx > 0 && x == world.GetWidth() - 1) {
      return false;
    }
    if (dy < 0 && y == 0) {
      return false;
    }
    if (dy > 0 && y == world.GetHeight() - 1) {
      return false;
    }
    auto& candidate = world.GetSquare(x + dx, y + dy);
    auto& material  = square.material;
    if (!candidate.material->is_rigid && candidate.material->mass < material->mass) {
      std::swap(square, candidate);

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
  FallingPhysics() : Physics(false) {}
};

class LiquidPhysics : public Physics {
public:
  LiquidPhysics() : Physics(true) {}
};


}  // namespace pixelengine::world