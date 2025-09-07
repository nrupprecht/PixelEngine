#pragma once

#include "pixelengine/utility/Vec2.h"
#include <array>
#include <cmath>

namespace pixelengine::math {

class Mat2 {
public:
  Mat2() = default;
  Mat2(float m00, float m01, float m10, float m11) : data_ {m00, m01, m10, m11} {}

  static Mat2 Identity() { return {1.f, 0.f, 0.f, 1.f}; }

  static Mat2 Scale(float sx, float sy) { return {sx, 0.f, 0.f, sy}; }

  static Mat2 Rotation(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    return Mat2 {c, -s, s, c};
  }

  static Mat2 Shear(float shx, float shy) { return {1.f, shx, shy, 1.f}; }

  float& operator()(std::size_t x, std::size_t y) {
    // PIXEL_ASSERT(x < 2, "x must be < 2, was " << x);
    // PIXEL_ASSERT(y < 2, "y must be < 2, was " << y);
    return data_[y * 2 + x];
  }

  friend Mat2 operator*(const Mat2& a, const Mat2& b);
  friend Mat2 operator*(float x, const Mat2& m);
  
  friend Vec2 operator*(const Mat2& m, const Vec2& v) {
    return {m.data_[0] * v.x + m.data_[1] * v.y, m.data_[2] * v.x + m.data_[3] * v.y};
  }

private:
  std::array<float, 4> data_;
};

inline Mat2 operator*(const Mat2& a, const Mat2& b) {
  Mat2 result {};

  result.data_[0] = a.data_[0] * b.data_[0] + a.data_[1] * b.data_[2];
  result.data_[1] = a.data_[0] * b.data_[1] + a.data_[1] * b.data_[3];
  result.data_[2] = a.data_[2] * b.data_[0] + a.data_[3] * b.data_[2];
  result.data_[3] = a.data_[2] * b.data_[1] + a.data_[3] * b.data_[3];

  return result;
}

inline Mat2 operator*(float x, const Mat2& m) {
  Mat2 result {};
  result.data_[0] = x * m.data_[0];
  result.data_[1] = x * m.data_[1];
  result.data_[2] = x * m.data_[2];
  result.data_[3] = x * m.data_[3];
  return result;
}

}  // namespace pixelengine::math