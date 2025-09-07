#pragma once

#include <array>
#include <cmath>

namespace pixelengine::math {

class Mat3 {
public:
  Mat3() = default;
  Mat3(float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
      : data_ {m00, m01, m02, m10, m11, m12, m20, m21, m22} {}

  static Mat3 Identity() { return Mat3 {1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f}; }

  static Mat3 Translation(float x, float y) { return Mat3 {1.f, 0.f, x, 0.f, 1.f, y, 0.f, 0.f, 1.f}; }

  static Mat3 Scale(float sx, float sy) { return Mat3 {sx, 0.f, 0.f, 0.f, sy, 0.f, 0.f, 0.f, 1.f}; }

  static Mat3 Rotation(float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);
    return Mat3 {c, -s, 0.f, s, c, 0.f, 0.f, 0.f, 1.f};
  }

  static Mat3 Shear(float shx, float shy) { return Mat3 {1.f, shx, 0.f, shy, 1.f, 0.f, 0.f, 0.f, 1.f}; }

  float& operator()(std::size_t x, std::size_t y) {
    // PIXEL_ASSERT(x < 3, "x must be < 3, was " << x);
    // PIXEL_ASSERT(y < 3, "y must be < 3, was " << y);
    return data_[y * 3 + x];
  }

  friend Mat3 operator*(const Mat3& a, const Mat3& b);
  friend Mat3 operator*(float x, const Mat3& m);

private:
  std::array<float, 9> data_;
};

inline Mat3 operator*(const Mat3& a, const Mat3& b) {
  Mat3 result {};

  result.data_[0] = a.data_[0] * b.data_[0] + a.data_[1] * b.data_[3] + a.data_[2] * b.data_[6];
  result.data_[1] = a.data_[0] * b.data_[1] + a.data_[1] * b.data_[4] + a.data_[2] * b.data_[7];
  result.data_[2] = a.data_[0] * b.data_[2] + a.data_[1] * b.data_[5] + a.data_[2] * b.data_[8];

  result.data_[3] = a.data_[3] * b.data_[0] + a.data_[4] * b.data_[3] + a.data_[5] * b.data_[6];
  result.data_[4] = a.data_[3] * b.data_[1] + a.data_[4] * b.data_[4] + a.data_[5] * b.data_[7];
  result.data_[5] = a.data_[3] * b.data_[2] + a.data_[4] * b.data_[5] + a.data_[5] * b.data_[8];

  result.data_[6] = a.data_[6] * b.data_[0] + a.data_[7] * b.data_[3] + a.data_[8] * b.data_[6];
  result.data_[7] = a.data_[6] * b.data_[1] + a.data_[7] * b.data_[4] + a.data_[8] * b.data_[7];
  result.data_[8] = a.data_[6] * b.data_[2] + a.data_[7] * b.data_[5] + a.data_[8] * b.data_[8];

  return result;
}

inline Mat3 operator*(float x, const Mat3& m) {
  Mat3 result {};
  result.data_[0] = x * m.data_[0];
  result.data_[1] = x * m.data_[1];
  result.data_[2] = x * m.data_[2];
  result.data_[3] = x * m.data_[3];
  result.data_[4] = x * m.data_[4];
  result.data_[5] = x * m.data_[5];
  result.data_[6] = x * m.data_[6];
  result.data_[7] = x * m.data_[7];
  result.data_[8] = x * m.data_[8];
  return result;
}

}