#pragma once

#include <string_view>

#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <Metal/Metal.hpp>

#include "pixelengine/graphics/TextureContainer.h"

namespace pixelengine::storage {

//! \brief Load a PNG file from disk into a Metal texture.
MTL::Texture* LoadMTLTextureFromPNG(MTL::Device* device, std::string_view path);

//! \brief Load a PNG file from disk into a TextureWrapper.
inline std::unique_ptr<graphics::TextureWrapper> LoadTextureFromPNG(MTL::Device* device,
                                                                    std::string_view path) {
  return std::make_unique<graphics::TextureWrapper>(LoadMTLTextureFromPNG(device, path));
}

}  // namespace pixelengine::storage