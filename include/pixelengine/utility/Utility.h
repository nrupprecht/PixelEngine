//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <Lightning/Lightning.h>
#include <cstdlib>  // For rand(), RAND_MAX
#include <simd/simd.h>

namespace pixelengine {

struct Dimensions {
  std::size_t width, height;

  std::size_t Area() const { return width * height; }
};

inline float randf() {
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

inline int randi(int min, int max) {
  return min + rand() % (max - min);
}

Dimensions GetScreenResolution();

namespace math {

constexpr simd::float3 add(const simd::float3& a, const simd::float3& b) {
  return {a.x + b.x, a.y + b.y, a.z + b.z};
}

constexpr simd_float4x4 makeIdentity() {
  using simd::float4;
  return (simd_float4x4) {(float4) {1.f, 0.f, 0.f, 0.f},
                          (float4) {0.f, 1.f, 0.f, 0.f},
                          (float4) {0.f, 0.f, 1.f, 0.f},
                          (float4) {0.f, 0.f, 0.f, 1.f}};
}

simd::float4x4 makePerspective(float fovRadians, float aspect, float znear, float zfar);
simd::float4x4 makeXRotate(float angleRadians);
simd::float4x4 makeYRotate(float angleRadians);
simd::float4x4 makeZRotate(float angleRadians);
simd::float4x4 makeTranslate(const simd::float3& v);
simd::float4x4 makeScale(const simd::float3& v);
simd::float3x3 discardTranslation(const simd::float4x4& m);

}  // namespace math

}  // namespace pixelengine