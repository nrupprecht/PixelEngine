#pragma once

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

#include "TextureBitmap.h"
#include "pixelengine/graphics/ShaderProgram.h"
#include "pixelengine/utility/AutoBuffer.h"
#include "pixelengine/utility/Utility.h"

namespace pixelengine::graphics {


//! \brief Represents a drawable object.
class Drawable {
public:
  explicit Drawable(ShaderProgram* shader_program);

  virtual ~Drawable() = default;

  void Draw(MTL::RenderCommandEncoder* cmd_encoder);

protected:
  //! \brief Update every texture.
  void updateTextures();

  //! \brief Set the arguments for the shader in the cmd encoder.
  void setArguments(MTL::RenderCommandEncoder* cmd_encoder);

  //! \brief Draw the object.
  virtual void draw(MTL::RenderCommandEncoder* cmd_encoder) = 0;

  std::vector<utility::AutoBuffer> buffers_;

  std::vector<utility::AutoBuffer> fragment_buffers_;

  std::vector<std::unique_ptr<TextureBitmap>> textures_;

  ShaderProgram* shader_program_ {};
};

}  // namespace pixelengine::graphics