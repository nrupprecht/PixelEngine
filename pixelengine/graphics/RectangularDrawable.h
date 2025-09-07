//
// Created by Nathaniel Rupprecht on 9/22/24.
//

#pragma once

#include <array>

#include "pixelengine/graphics/Drawable.h"
#include "pixelengine/graphics/ShaderProgram.h"

namespace pixelengine::graphics {

//! \brief A rectangular drawable object.
class RectangularDrawable : public Drawable {
public:
  RectangularDrawable(ShaderProgram* shader_program,
                      float width,
                      float height,
                      std::unique_ptr<TextureContainer> texture);

  RectangularDrawable(ShaderProgram* shader_program,
                      float width,
                      float height,
                      std::size_t texture_width,
                      std::size_t texture_height);

  [[nodiscard]] TextureContainer& GetTextureBitmap() const;

  void SetPosition(Vec2 position);
  void SetPosition(float x, float y);

  [[nodiscard]] std::array<float, 2> GetPosition() const;

  void SetWidth(float width);
  void SetHeight(float height);

  //! \brief Replace the current texture with a new one, returning the old one.
  [[nodiscard]] std::unique_ptr<TextureContainer> SwapTextures(std::unique_ptr<TextureContainer> new_texture);

private:
  void drawVertices(MTL::RenderCommandEncoder* cmd_encoder,
                    application::WindowContext* context,
                    Vec2 parent_offset) override;

  //! \brief Regenerate the vertex positions. Called when the width or height changes.
  void generateVertices(bool update = true);

  Vec2 position_;
  float width_ {}, height_ {};

  utility::AutoBuffer index_buffer_;
  std::array<shadertypes::VertexData, 4> verts_ {};
};

}  // namespace pixelengine::graphics