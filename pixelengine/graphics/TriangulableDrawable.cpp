#include "pixelengine/graphics/TriangulableDrawable.h"
// Other files.
#include "pixelengine/graphics/ShaderStore.h"

namespace pixelengine::graphics {

TriangulableDrawable::TriangulableDrawable(std::vector<Vec2> vertices, std::vector<uint16_t> indices, Color color)
    : Drawable(graphics::ShaderStore::GetInstance()->GetShaderProgram("ColorShader"))
    , vertices_(std::move(vertices)) {
  auto device = shader_program_->GetDevice();
  simd::float4 col = color.ToFloat4();  // Red color

  // Buffer 0 is the vertices (array of float2).
  // Buffer 1 is the color (float4).
  buffers_.emplace_back(utility::AutoBuffer::New<Vec2>(device, vertices_.data(), vertices_.size()));
  buffers_.emplace_back(utility::AutoBuffer::New<simd::float4>(device, &col, 1));

  index_buffer_ = utility::AutoBuffer::New<uint16_t>(device, indices.data(), indices.size());
}

void TriangulableDrawable::drawVertices(MTL::RenderCommandEncoder* cmd_encoder,
                                        application::WindowContext* context,
                                        Vec2 parent_offset) {
  auto displaced_verts = vertices_;
  // Displace vertices.
  for (auto& vert : displaced_verts) {
    Vec2 pos = Vec2 {vert.x, vert.y} + parent_offset;
    pos      = context ? context->TranslatePoint(pos) : pos;
    vert.x   = pos.x;
    vert.y   = pos.y;
  }
  // TODO: Only update when necessary.
  buffers_.at(0).CopyInto<Vec2>(displaced_verts.data());
  
  cmd_encoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                     index_buffer_.Size(),
                                     MTL::IndexTypeUInt16,
                                     index_buffer_.Data(),
                                     0);
}

}  // namespace pixelengine::graphics