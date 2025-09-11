//
// Created by Nathaniel Rupprecht on 9/22/24.
//

#include "pixelengine/graphics/RectangularDrawable.h"

#include "pixelengine/utility/Contracts.h"

namespace pixelengine::graphics {

RectangularDrawable::RectangularDrawable(ShaderProgram* shader_program,
                                         float width,
                                         float height,
                                         std::unique_ptr<TextureContainer> texture)
    : Drawable(shader_program)
    , width_(width)
    , height_(height) {
  generateVertices(false);

  uint16_t indices[] = {0, 1, 2, 0, 2, 3};

  auto device = shader_program->GetDevice();

  buffers_.emplace_back(utility::AutoBuffer::New<shadertypes::VertexData>(device, verts_.data(), 4));
  // Allocate the main texture.
  if (texture) {
    textures_.emplace_back(std::move(texture));
  }
  // Set index buffer data.
  index_buffer_ = utility::AutoBuffer::New<uint16_t>(device, indices, 6);
}

RectangularDrawable::RectangularDrawable(ShaderProgram* shader_program,
                                         float width,
                                         float height,
                                         std::size_t texture_width,
                                         std::size_t texture_height)
    : RectangularDrawable(
          shader_program,
          width,
          height,
          std::make_unique<TextureBitmapOwning>(texture_width, texture_height, shader_program->GetDevice())) {
}

TextureContainer& RectangularDrawable::GetTextureBitmap() const {
  return *textures_.at(0);
}

void RectangularDrawable::SetWidth(float width) {
  width_ = width;
  generateVertices();
}

void RectangularDrawable::SetHeight(float height) {
  height_ = height;
  generateVertices();
}


std::unique_ptr<TextureContainer> RectangularDrawable::SwapTextures(
    std::unique_ptr<TextureContainer> new_texture) {
  PIXEL_REQUIRE(textures_.size() == 1, "there should be a single texture in the RectangularDrawable");
  auto old_texture = std::move(textures_.at(0));
  textures_.at(0)  = std::move(new_texture);
  return old_texture;
}

void RectangularDrawable::_onUpdatedTransform(const math::Transformation2D& transformation) {
  LOG_SEV(Major) << "Transformation: " << transformation;
  // Update the vertex positions based on the current transformation.
  for (auto& vert : verts_) {
    LOG_SEV(Major) << "Position was: " << float(vert.position.x) << ", " << float(vert.position.y);
    vert.position = transformation.TransformPoint(vert.position);

    LOG_SEV(Major) << "Updated vertex position: " << float(vert.position.x) << ", " << float(vert.position.y);
  }
  // Update the position buffer.
  buffers_.at(0).CopyInto<shadertypes::VertexData>(verts_.data());
}

void RectangularDrawable::drawVertices(MTL::RenderCommandEncoder* cmd_encoder) {
  cmd_encoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                     index_buffer_.Size(),
                                     MTL::IndexTypeUInt16,
                                     index_buffer_.Data(),
                                     0);
}

void RectangularDrawable::generateVertices(bool update) {
  // texcoord is the texture coordinates.
  verts_[0] = {.position = simd::float3 {0.f, +height_, 0.0f}, .texcoord = {0.f, 0.f}};
  verts_[1] = {.position = simd::float3 {0.f, 0.f, 0.0f}, .texcoord = {0.f, 1.f}};
  verts_[2] = {.position = simd::float3 {+width_, 0.f, 0.0f}, .texcoord = {1.f, 1.f}};
  verts_[3] = {.position = simd::float3 {+width_, +height_, 0.0f}, .texcoord = {1.f, 0.f}};

  if (update) {
    buffers_.at(0).CopyInto<shadertypes::VertexData>(verts_.data());
  }
}

}  // namespace pixelengine::graphics