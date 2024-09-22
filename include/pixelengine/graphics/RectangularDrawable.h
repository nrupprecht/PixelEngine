//
// Created by Nathaniel Rupprecht on 9/22/24.
//

#pragma once

#include <array>

#include "pixelengine/graphics/Drawable.h"

namespace pixelengine::graphics {

namespace shadertypes {

struct VertexData {
  simd::float3 position;
  simd::float2 texcoord;
};

}  // namespace shadertypes

//! \brief A rectangular drawable object.
class RectangularDrawable : public Drawable {
public:
  RectangularDrawable(std::unique_ptr<PipelineState> pipeline_state,
                      std::size_t texture_width,
                      std::size_t texture_height,
                      MTL::Device* device);

  [[nodiscard]] TextureBitmap& GetTextureBitmap() const;

  void SetPosition(float x, float y);

  [[nodiscard]] std::array<float, 2> GetPosition() const;

private:
  void draw(MTL::RenderCommandEncoder* cmd_encoder) override;

  utility::AutoBuffer index_buffer_;

  float x_ {}, y_ {};

  std::array<shadertypes::VertexData, 4> verts_{};
};

}  // namespace pixelengine::graphics