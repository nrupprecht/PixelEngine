//
// Created by Nathaniel Rupprecht on 9/29/24.
//

#pragma once

namespace pixelengine {

//! \brief Two dimensional vector.
struct Vec2 {
  float x {}, y {};
};

// ===========================================================================================================
//  Mathematical operations.
// ===========================================================================================================

inline Vec2 operator*(float x, const Vec2& v) {
  return {x * v.x, x * v.y};
}

inline Vec2 operator*(const Vec2& v, float x) {
  return {x * v.x, x * v.y};
}

inline Vec2 operator+(const Vec2& a, const Vec2& b) {
  return {a.x + b.x, a.y + b.y};
}

inline Vec2 operator-(const Vec2& a, const Vec2& b) {
  return {a.x - b.x, a.y - b.y};
}

inline Vec2 operator/(const Vec2& a, float x) {
  return {a.x / x, a.y / x};
}

inline Vec2& operator+=(Vec2& a, const Vec2& b) {
  a.x += b.x;
  a.y += b.y;
  return a;
}

inline Vec2& operator-=(Vec2& a, const Vec2& b) {
  a.x -= b.x;
  a.y -= b.y;
  return a;
}

inline float operator*(Vec2& a, const Vec2& b) {
  return a.x * b.x + a.y * b.y;
}

}  // namespace pixelengine