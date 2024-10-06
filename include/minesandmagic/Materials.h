//
// Created by Nathaniel Rupprecht on 10/5/24.
//

#pragma once


#include "pixelengine/world/World.h"

namespace minesandmagic {

inline constexpr pixelengine::world::FallingPhysics falling {};
inline constexpr pixelengine::world::LiquidPhysics liquid {};
inline constexpr pixelengine::world::Stationary stationary {};

using pixelengine::world::SAND;
using pixelengine::world::DIRT;
using pixelengine::world::WATER;
using pixelengine::world::Square;

constexpr pixelengine::Color SAND_COLORS[] = {
  pixelengine::Color(204, 171, 114),
  pixelengine::Color(200, 168, 113),
  pixelengine::Color(179, 149, 100),
  pixelengine::Color(179, 149, 100),
};

// A background color.
constexpr pixelengine::Color BACKGROUND(240, 228, 228);

}  // namespace minesandmagic