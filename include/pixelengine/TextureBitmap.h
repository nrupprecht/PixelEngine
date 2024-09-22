//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <iostream>

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>

#include "Color.h"

namespace pixelengine {

//! \brief A texture that can be set like a bitmap.
class TextureBitmap {
public:
  TextureBitmap() = default;

  TextureBitmap(size_t width, size_t height, MTL::Device* device);

  ~TextureBitmap() { texture_->release(); }

  TextureBitmap& operator=(TextureBitmap&& other) noexcept;

  void Initialize(std::size_t width, std::size_t height, MTL::Device* device);

  [[nodiscard]] const MTL::Texture* GetTexture() const { return texture_; }

  void SetPixel(std::size_t x, std::size_t y, const Color& color);

  [[nodiscard]] Color GetPixel(std::size_t x, std::size_t y) const;

  [[nodiscard]] std::size_t GetWidth() const { return width_; }

  [[nodiscard]] std::size_t GetHeight() const { return height_; }

  void Update();

private:
  void initialize(MTL::Device* device);

  [[nodiscard]] MTL::TextureDescriptor* createTextureDescriptor() const;

  [[nodiscard]] const uint8_t* getData() const { return reinterpret_cast<const uint8_t*>(pixel_data.data()); }

  void update();

  //! \brief The texture width.
  std::size_t width_ = 0;
  //! \brief The texture height.
  std::size_t height_ = 0;

  //! \brief Pointer to the texture.
  MTL::Texture* texture_ = nullptr;

  //! \brief The buffer of pixel data.
  std::vector<Color> pixel_data;

  //! \brief Whether the buffer has been changed since the last call to `update()`.
  bool is_dirty_ = false;
};

}  // namespace pixelengine