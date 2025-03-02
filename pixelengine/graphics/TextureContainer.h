//
// Created by Nathaniel Rupprecht on 9/22/24.
//

#pragma once

#include "pixelengine/graphics/TextureBitmap.h"

namespace pixelengine::graphics {

//! \brief Object that is able to return a texture.
//!
//! This allows it to either own a texture, or reference a texture that it does not own.
class TextureContainer {
public:
  virtual ~TextureContainer() = default;
  virtual void Update() {}
  [[nodiscard]] virtual MTL::Texture* GetTexture() = 0;
};

//! \brief TextureContainer that owns a TextureBitmap.
class TextureBitmapOwning : public TextureContainer {
public:
  TextureBitmapOwning(std::size_t texture_width, std::size_t texture_height, MTL::Device* device)
      : texture_bitmap_(texture_width, texture_height, device) {}
  void Update() override { texture_bitmap_.Update(); }
  [[nodiscard]] MTL::Texture* GetTexture() override { return texture_bitmap_.GetTexture(); }

  TextureBitmap& GetTextureBitmap() { return texture_bitmap_; }
private:
  TextureBitmap texture_bitmap_;
};

//! \brief TextureContainer that does not own the texture.
class TextureWrapper : public TextureContainer {
public:
  explicit TextureWrapper(MTL::Texture* texture) : texture_(texture) {}
  [[nodiscard]] MTL::Texture* GetTexture() override { return texture_; }
private:
  MTL::Texture* texture_;
};

}  // namespace pixelengine::graphics
