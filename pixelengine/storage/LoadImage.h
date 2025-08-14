#pragma once

#include <string_view>

#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <Metal/Metal.hpp>

#include "pixelengine/graphics/TextureContainer.h"

namespace pixelengine::storage {

// Got this code from chat-gpt, doesn't seem to work correctly.
MTL::Texture* LoadMTLTextureFromImage(MTL::Device* device, std::string_view filepath);

//! \brief Load a BMP file from disk into a TextureWrapper.
inline std::unique_ptr<graphics::TextureWrapper> LoadTextureFromImage(MTL::Device* device,
                                                                      std::string_view filepath) {
  return std::make_unique<graphics::TextureWrapper>(LoadMTLTextureFromImage(device, filepath));
}

}  // namespace pixelengine::storage