//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <cstdint>
#include <algorithm>

namespace pixelengine {

//! \brief A structure that represents a color.
struct alignas(uint32_t) Color {
  Color() = default;

  constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF) : red(r), green(g), blue(b), alpha(a) {}

  static Color FromFloats(float r, float g, float b, float a = 1.0f) {
    r = std::max(0.f, std::min(r, 1.f));
    g = std::max(0.f, std::min(g, 1.f));
    b = std::max(0.f, std::min(b, 1.f));
    a = std::max(0.f, std::min(a, 1.f));
    return {static_cast<uint8_t>(255 * r),
            static_cast<uint8_t>(255 * g),
            static_cast<uint8_t>(255 * b),
            static_cast<uint8_t>(255 * a)};
  }

  uint8_t red = 0xFF, green = 0xFF, blue = 0xFF, alpha = 0xFF;

  uint32_t ToUInt32() { return *reinterpret_cast<uint32_t*>(&red); }
};

}  // namespace pixelengine