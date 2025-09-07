//
// Created by Nathaniel Rupprecht on 9/29/24.
//

#pragma once

#include <array>
#include <ostream>

namespace pixelengine {

//! \brief Two dimensional vector.
template<typename T>
struct BaseVec2 {
  BaseVec2(T a, T b) : x(a), y(b) {}
  BaseVec2(std::array<T, 2> arr) : x(arr[0]), y(arr[1]) {}
  BaseVec2() = default;

  T x {}, y {};

  bool operator==(const BaseVec2& other) const { return x == other.x && y == other.y; }

  //! \brief Convert to a different type of vector.
  template<typename S>
  BaseVec2<S> To() const noexcept(std::is_nothrow_convertible_v<T, S>)
    requires(std::is_convertible_v<T, S>)
  {
    return {static_cast<S>(x), static_cast<S>(y)};
  }

  friend std::ostream& operator<<(std::ostream& os, const BaseVec2& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
  }
};

// Floating point vector.
using Vec2 = BaseVec2<float>;

//! \brief Integral position vector.
using PVec2 = BaseVec2<long long>;

// ===========================================================================================================
//  Mathematical operations.
// ===========================================================================================================

template<typename T>
BaseVec2<T> operator*(float x, const BaseVec2<T>& v) {
  return {x * v.x, x * v.y};
}

template<typename T, typename S>
auto operator*(const BaseVec2<T>& v, S x) -> BaseVec2<decltype(v.x * x)> {
  return {x * v.x, x * v.y};
}

template<typename T>
BaseVec2<T> operator+(const BaseVec2<T>& a, const BaseVec2<T>& b) {
  return {a.x + b.x, a.y + b.y};
}

template<typename T>
BaseVec2<T> operator-(const BaseVec2<T>& a, const BaseVec2<T>& b) {
  return {a.x - b.x, a.y - b.y};
}

template<typename T, typename S>
auto operator/(const BaseVec2<T>& a, S x) -> BaseVec2<decltype(a.x / x)> {
  return {a.x / x, a.y / x};
}

template<typename T>
BaseVec2<T>& operator+=(BaseVec2<T>& a, const BaseVec2<T>& b) {
  a.x += b.x;
  a.y += b.y;
  return a;
}

template<typename T>
BaseVec2<T>& operator-=(BaseVec2<T>& a, const BaseVec2<T>& b) {
  a.x -= b.x;
  a.y -= b.y;
  return a;
}

//! \brief Dot product of two Vec2's.
template<typename T>
auto operator*(const BaseVec2<T>& a, const BaseVec2<T>& b) {
  return a.x * b.x + a.y * b.y;
}

//! \brief Cross product of two Vec2's.
template<typename T>
T operator^(const BaseVec2<T>& a, const BaseVec2<T>& b) {
  return a.x * b.y - a.y * b.x;
}

//! \brief Add two Vec2's and get the PVec2 and a remainder.
inline std::pair<PVec2, Vec2> AddWithRemainder(const PVec2& pvec, const Vec2& vec) {
  auto pvec_out  = pvec;
  auto remainder = vec;
  if (0 < vec.x) {
    pvec_out.x += std::lroundf(std::floor(vec.x));
    remainder.x -= std::floor(vec.x);
  }
  else {
    pvec_out.x += std::lroundf(std::ceil(vec.x));
    remainder.x -= std::ceil(vec.x);
  }
  // Y
  if (0 < vec.y) {
    pvec_out.y += std::lroundf(std::floor(vec.y));
    remainder.y -= std::floor(vec.y);
  }
  else {
    pvec_out.y += std::lroundf(std::ceil(vec.y));
    remainder.y -= std::ceil(vec.y);
  }

  return {pvec_out, remainder};
}

inline float length(const Vec2& v) {
  return std::sqrt(v.x * v.x + v.y * v.y);
}

inline Vec2 normalize(const Vec2& v) {
  float len = length(v);
  return {v.x / len, v.y / len};
}

inline bool WithinRectangle(Vec2 rA, Vec2 rB, Vec2 point) {
  if (rB.x < rA.x) std::swap(rA.x, rB.x);
  if (rB.y < rA.y) std::swap(rA.y, rB.y);
  return rA.x <= point.x && point.x <= rB.x && rA.y <= point.y && point.y <= rB.y;
}

}  // namespace pixelengine