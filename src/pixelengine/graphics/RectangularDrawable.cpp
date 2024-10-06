//
// Created by Nathaniel Rupprecht on 9/22/24.
//

#include "pixelengine/graphics/RectangularDrawable.h"

namespace pixelengine::graphics {

RectangularDrawable::RectangularDrawable(ShaderProgram* shader_program,
                                         std::unique_ptr<TextureContainer> texture)
    : Drawable(shader_program)
    , width_(2.f)
    , height_(2.f) {
  generateVertices(false);

  uint16_t indices[] = {0, 1, 2, 0, 2, 3};

  auto device = shader_program->GetDevice();

  buffers_.emplace_back(utility::AutoBuffer::New<shadertypes::VertexData>(device, verts_.data(), 4));
  // Allocate the main texture.
  textures_.emplace_back(std::move(texture));
  // Set index buffer data.
  index_buffer_ = utility::AutoBuffer::New<uint16_t>(device, indices, 6);
}


RectangularDrawable::RectangularDrawable(ShaderProgram* shader_program,
                                         std::size_t texture_width,
                                         std::size_t texture_height)
    : RectangularDrawable(
          shader_program,
          std::make_unique<TextureBitmapOwning>(texture_width, texture_height, shader_program->GetDevice())) {
}

TextureContainer& RectangularDrawable::GetTextureBitmap() const {
  return *textures_.at(0);
}

void RectangularDrawable::SetPosition(float x, float y) {
  auto [ave_x, ave_y] = GetPosition();

  auto dx = x - ave_x, dy = y - ave_y;
  for (auto& vert : verts_) {
    vert.position.x += dx;
    vert.position.y += dy;
  }
  // Update the position buffer.
  buffers_.at(0).CopyInto<shadertypes::VertexData>(verts_.data());
}

std::array<float, 2> RectangularDrawable::GetPosition() const {
  auto ave_x = 0.5f * (verts_[0].position.x + verts_[2].position.x);
  auto ave_y = 0.5f * (verts_[0].position.y + verts_[2].position.y);
  return {ave_x, ave_y};
}

void RectangularDrawable::SetWidth(float width) {
  width_ = width;
  generateVertices();
}

void RectangularDrawable::SetHeight(float height) {
  height_ = height;
  generateVertices();
}

void RectangularDrawable::drawVertices(MTL::RenderCommandEncoder* cmd_encoder, Vec2 parent_offset) {
  // TODO: Update vertices.

  cmd_encoder->drawIndexedPrimitives(
      MTL::PrimitiveType::PrimitiveTypeTriangle, 6, MTL::IndexTypeUInt16, index_buffer_.Data(), 0);
}

void RectangularDrawable::generateVertices(bool update) {
  verts_[0] = {.position = simd::float3 {-width_ / 2, +height_ / 2, 0.0f}, .texcoord = {0.f, 0.f}};
  verts_[1] = {.position = simd::float3 {-width_ / 2, -height_ / 2, 0.0f}, .texcoord = {0.f, 1.f}};
  verts_[2] = {.position = simd::float3 {+width_ / 2, -height_ / 2, 0.0f}, .texcoord = {1.f, 1.f}};
  verts_[3] = {.position = simd::float3 {+width_ / 2, +height_ / 2, 0.0f}, .texcoord = {1.f, 0.f}};

  if (update) {
    buffers_.at(0).CopyInto<shadertypes::VertexData>(verts_.data());
  }
}

}  // namespace pixelengine::graphics