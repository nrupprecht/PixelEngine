//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#include "../../../include/pixelengine/graphics/TextureBitmap.h"
// Other files.

namespace pixelengine {
TextureBitmap::TextureBitmap(size_t width, size_t height, MTL::Device* device)
    : width_(width)
    , height_(height)
    , pixel_data(width * height) {
  initialize(device);
}

TextureBitmap& TextureBitmap::operator=(TextureBitmap&& other) noexcept {
  if (texture_) {
    texture_->release();
  }
  texture_   = other.texture_;
  width_     = other.width_;
  height_    = other.height_;
  is_dirty_  = other.is_dirty_;
  pixel_data = std::move(other.pixel_data);

  other.texture_  = nullptr;
  other.width_    = 0;
  other.height_   = 0;
  other.is_dirty_ = false;

  update();

  return *this;
}

// TextureBitmap::~TextureBitmap() {
//   texture_->release();
// }

void TextureBitmap::Initialize(std::size_t width, std::size_t height, MTL::Device* device) {
  if (texture_) {
    texture_->release();
  }
  width_  = width;
  height_ = height;
  pixel_data.resize(width_ * height_);

  initialize(device);
}

void TextureBitmap::SetPixel(std::size_t x, std::size_t y, const Color& color) {
  assert(x < width_ && y < height_);  // TODO: Other assert type.
  auto index        = y * width_ + x;
  pixel_data[index] = color;
  is_dirty_         = true;
}

Color TextureBitmap::GetPixel(std::size_t x, std::size_t y) const {
  assert(x < width_ && y < height_);  // TODO: Other assert type.
  auto index = y * width_ + x;
  return pixel_data[index];
}

void TextureBitmap::Update() {
  if (is_dirty_) {
    update();
  }
}

void TextureBitmap::initialize(MTL::Device* device) {
  auto texture_descriptor = createTextureDescriptor();
  texture_                = device->newTexture(texture_descriptor);
  texture_descriptor->release();

  update();
}

MTL::TextureDescriptor* TextureBitmap::createTextureDescriptor() const {
  MTL::TextureDescriptor* pTextureDesc = MTL::TextureDescriptor::alloc()->init();
  pTextureDesc->setWidth(width_);
  pTextureDesc->setHeight(height_);
  pTextureDesc->setPixelFormat(MTL::PixelFormatRGBA8Unorm);
  pTextureDesc->setTextureType(MTL::TextureType2D);
  pTextureDesc->setStorageMode(MTL::StorageModeManaged);
  pTextureDesc->setUsage(MTL::ResourceUsageSample | MTL::ResourceUsageRead);
  return pTextureDesc;
}

void TextureBitmap::update() {
  // TODO: Could detect a more specific region that needs to be updated.
  MTL::Region region(0, 0, 0, width_, height_, 1);
  texture_->replaceRegion(region, 0, getData(), width_ * 4);
  is_dirty_ = false;
}

}  // namespace pixelengine