#pragma once

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

#include "pixelengine/TextureBitmap.h"
#include "pixelengine/graphics/PipelineState.h"
#include "pixelengine/utility/AutoBuffer.h"
#include "pixelengine/utility/Utility.h"

namespace pixelengine::graphics {

class TextureBox {
public:
  virtual ~TextureBox() = default;
  virtual void Update() {}
  [[nodiscard]] virtual const MTL::Texture* GetTexture() const = 0;
};

class RawTextureBox : public TextureBox {
public:
  explicit RawTextureBox(MTL::Texture* texture) : texture_(texture) {}
  [[nodiscard]] const MTL::Texture* GetTexture() const override { return texture_; }
private:
  MTL::Texture* texture_;
};

class TextureBitmapBox : public TextureBox {
public:
  explicit TextureBitmapBox(std::unique_ptr<TextureBitmap> texture_bitmap)
      : texture_bitmap_(std::move(texture_bitmap)) {}
  void Update() override { texture_bitmap_->Update(); }
  [[nodiscard]] const MTL::Texture* GetTexture() const override { return texture_bitmap_->GetTexture(); }
private:
  std::unique_ptr<TextureBitmap> texture_bitmap_;
};


//! \brief Represents a drawable object.
class Drawable {
public:
  explicit Drawable(std::unique_ptr<PipelineState> pipeline_state);

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

  std::unique_ptr<PipelineState> pipeline_state_ {};
};

}  // namespace pixelengine::graphics