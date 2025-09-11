#include "pixelengine/graphics/TriangulableDrawable.h"
// Other files.
#include "pixelengine/graphics/ShaderStore.h"

namespace pixelengine::graphics {

TriangulableDrawable::TriangulableDrawable(std::vector<Vec2> vertices,
                                           std::vector<uint16_t> indices,
                                           Color color)
    : Drawable(graphics::ShaderStore::GetInstance()->GetShaderProgram("ColorShader"))
    , vertices_(std::move(vertices)) {
  auto device      = shader_program_->GetDevice();
  simd::float4 col = color.ToFloat4();

  // Buffer 0 is the vertices (array of float2).
  // Buffer 1 is the color (float4).
  buffers_.emplace_back(utility::AutoBuffer::New<Vec2>(device, vertices_.data(), vertices_.size()));
  buffers_.emplace_back(utility::AutoBuffer::New<simd::float4>(device, &col, 1));

  index_buffer_ = utility::AutoBuffer::New<uint16_t>(device, indices.data(), indices.size());
}

void TriangulableDrawable::_onUpdatedTransform(const math::Transformation2D& transformation) {
  // Update the vertex positions based on the current transformation.
  for (auto& vert : vertices_) {
    vert = transformation.TransformPoint(vert);
  }
  // Update the position buffer.
  buffers_.at(0).CopyInto<Vec2>(vertices_.data());
}

void TriangulableDrawable::drawVertices(MTL::RenderCommandEncoder* cmd_encoder) {
  cmd_encoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                     index_buffer_.Size(),
                                     MTL::IndexTypeUInt16,
                                     index_buffer_.Data(),
                                     0);
}

}  // namespace pixelengine::graphics