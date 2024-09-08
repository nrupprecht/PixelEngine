//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <vector>

#include <Lightning/Lightning.h>

#include "pixelengine/Color.h"
#include "pixelengine/Utility.h"

namespace pixelengine::world {


//! \brief The physical properties of a material
struct Material {
  //! \brief The mass of the material.
  float mass = 1.0;
  //! \brief "Terminal velocity"
  float max_speed = 8.f;

  //! \brief Whether or not the material is fixed in place.
  bool is_rigid = false;
};

constexpr Material AIR {};
constexpr Material SAND {2.0};
constexpr Material WATER {1.5};

class SquareBehavior {
public:
  virtual ~SquareBehavior()                                                             = default;
  virtual void Update(float dt, std::size_t x, std::size_t y, class World& world) const = 0;
};

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

  //! \brief The velocity in the y direction.
  float velocity_y = 0.f;

  //! \brief For later - the temperature, in kelvin.
  float temperature = 300.f;



  void UpdateKinematics(float dt, float gravity) {
    velocity_y += dt * gravity;
    velocity_y = std::max(-material->max_speed, std::min(material->max_speed, velocity_y));
  }
};


class World {
public:
  World(std::size_t chunk_width, std::size_t chunk_height)
      : chunk_width_(chunk_width)
      , chunk_height_(chunk_height)
      , squares_(chunk_width_ * chunk_height_) {}

  [[nodiscard]] const Square& GetSquare(std::size_t x, std::size_t y) const { return getSquare(x, y); }
  [[nodiscard]] Square& GetSquare(std::size_t x, std::size_t y) { return getSquare(x, y); }

  void SetSquare(std::size_t x, std::size_t y, const Square& square) { getSquare(x, y) = square; }

  void Update(float dt) { evolve(dt); }

  [[nodiscard]] std::size_t GetWidth() const { return chunk_width_; }
  [[nodiscard]] std::size_t GetHeight() const { return chunk_height_; }

  [[nodiscard]] float GetGravity() const { return gravity_; }

private:
  std::size_t chunk_width_;
  std::size_t chunk_height_;

  float gravity_ = -10.;

  void evolve(float dt) {
    // Update motion.
    for (auto y = 0; y < chunk_height_; ++y) {
      for (auto x = 0; x < chunk_width_; ++x) {
        auto&& square = getSquare(x, y);
        if (!square.is_occupied || square.material->is_rigid || !square.behavior) {
          continue;
        }

        square.UpdateKinematics(dt, gravity_);
        square.behavior->Update(dt, x, y, *this);
      }
    }

    // TODO: Other updates, e.g. temperature, objects catching fire, reacting, etc.
  }

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

class Physics : public SquareBehavior {
public:
  Physics(bool allow_sideways) : allow_sideways_(allow_sideways) {}

  void Update(float dt, std::size_t x, std::size_t y, World& world) const override {
    auto& square = world.GetSquare(x, y);

    auto v = std::fabsf(square.velocity_y);

    bool is_blocked = false;
    while (1.f <= v) {
      std::tie(x, y, is_blocked) = singleUpdate(x, y, v, world);
    }
    // Random chance for one more update.
    if (!is_blocked && randf() < v - std::floorf(v)) {
      std::tie(x, y, is_blocked) = singleUpdate(x, y, v, world);
    }
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


class FallingBehavior : public SquareBehavior {
public:
  void Update(float dt, std::size_t x, std::size_t y, World& world) const override {
    auto& square = world.GetSquare(x, y);

    auto v           = std::fabsf(square.velocity_y);
    auto remainder   = v - std::floorf(v);
    auto num_updates = static_cast<int>(v) + (randf() < remainder);
    bool is_blocked  = false;
    for (auto i = 0; i < num_updates && !is_blocked; ++i) {
      std::tie(x, y, is_blocked) = singleUpdate(x, y, world);
    }
  }

private:
  std::tuple<std::size_t, std::size_t, bool> singleUpdate(std::size_t x, std::size_t y, World& world) const {
    if (y == 0) {
      return {x, y, true};
    }
    auto width = world.GetWidth();

    auto& square = world.GetSquare(x, y);

    auto&& below = world.GetSquare(x, y - 1);
    if (!below.is_occupied) {
      std::swap(square, below);
      return {x, y - 1, false};
    }

    // TODO: Randomize checking left and right.
    if (x > 0) {
      auto&& left = world.GetSquare(x - 1, y - 1);
      if (!left.is_occupied) {
        std::swap(square, left);
        return {x - 1, y - 1, false};
      }
    }
    if (x < width - 1) {
      auto&& right = world.GetSquare(x + 1, y - 1);
      if (!right.is_occupied) {
        std::swap(square, right);
        return {x + 1, y - 1, false};
      }
    }
    return {x, y, true};
  }
};

class LiquidBehavior : public SquareBehavior {
public:
  void Update(float dt, std::size_t x, std::size_t y, World& world) const override {
    auto& square = world.GetSquare(x, y);

    auto v           = std::fabsf(square.velocity_y);
    auto remainder   = v - std::floorf(v);
    auto num_updates = static_cast<int>(v) + (randf() < remainder);
    bool is_blocked  = false;
    for (auto i = 0; i < num_updates && !is_blocked; ++i) {
      std::tie(x, y, is_blocked) = singleUpdate(x, y, world);
    }
  }

private:
  std::tuple<std::size_t, std::size_t, bool> singleUpdate(std::size_t x, std::size_t y, World& world) const {
    // if (y == 0) {
    //   return {x, y, true};
    // }
    auto& square = world.GetSquare(x, y);

    // auto&& below = world.GetSquare(x, y - 1);
    // if (!below.is_occupied) {
    //   std::swap(square, below);
    //   return {x, y - 1, false};
    // }

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
    if (!candidate.is_occupied) {
      std::swap(square, candidate);
      return true;
    }
    return false;
  }
};

}  // namespace pixelengine::world