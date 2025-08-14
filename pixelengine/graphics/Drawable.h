#pragma once

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

#include "pixelengine/graphics/ShaderProgram.h"
#include "pixelengine/graphics/TextureContainer.h"
#include "pixelengine/node/Node.h"
#include "pixelengine/utility/AutoBuffer.h"
#include "pixelengine/utility/Utility.h"

namespace pixelengine::graphics {


//! \brief Represents a drawable object.
class Drawable : public Node {
public:
  explicit Drawable(ShaderProgram* shader_program);

protected:
  //! \brief Update every texture.
  void updateTextures();

  //! \brief Set the arguments for the shader in the cmd encoder.
  void setArguments(MTL::RenderCommandEncoder* cmd_encoder);

  void _draw(MTL::RenderCommandEncoder* cmd_encoder,
             application::WindowContext* context,
             Vec2 parent_offset) override;

  //! \brief Draw the object.
  virtual void drawVertices(MTL::RenderCommandEncoder* cmd_encoder,
                            application::WindowContext* context,
                            Vec2 parent_offset) = 0;

  std::vector<utility::AutoBuffer> buffers_;

  std::vector<utility::AutoBuffer> fragment_buffers_;

  std::vector<std::unique_ptr<TextureContainer>> textures_;

  ShaderProgram* shader_program_ {};
};

}  // namespace pixelengine::graphics