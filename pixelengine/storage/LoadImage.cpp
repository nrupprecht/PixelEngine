


#include <fstream>
#include <vector>

#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <Lightning/Lightning.h>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>


namespace pixelengine::storage {


// Loads an image file into an MTL::Texture
MTL::Texture* LoadMTLTextureFromImage(MTL::Device* device, std::string_view path) {
  // Load the image using CoreGraphics (works for PNG, JPEG, BMP, etc.)
  const CFStringRef pathCF         = CFStringCreateWithCString(NULL, path.data(), kCFStringEncodingUTF8);
  const CFURLRef url               = CFURLCreateWithFileSystemPath(NULL, pathCF, kCFURLPOSIXPathStyle, false);
  
  // Load image
  const CGImageSourceRef imgSource = CGImageSourceCreateWithURL(url, NULL);
  if (!imgSource) {
    LOG_SEV(Warning) << "Failed to load image '" << path << "'";
    return nullptr;
  }
  const CGImageRef image = CGImageSourceCreateImageAtIndex(imgSource, 0, NULL);
  if (!image) {
    LOG_SEV(Warning) << "Failed to create image from source for path: " << path;
    CFRelease(imgSource);
    return nullptr;
  }

  const std::size_t width = CGImageGetWidth(image);
  const std::size_t height = CGImageGetHeight(image);

  // Create raw RGBA pixel buffer
  std::vector<uint8_t> pixels(width * height * 4);
  CGContextRef context = CGBitmapContextCreate(pixels.data(),
                                               width,
                                               height,
                                               8,
                                               width * 4,
                                               CGImageGetColorSpace(image),
                                               kCGImageAlphaPremultipliedLast);

  // Draw image into context (flips vertically by default)
  CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);

  // Create Metal texture
  MTL::TextureDescriptor* texDesc =
      MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA8Unorm, 
        width, 
        height, 
        false);
  texDesc->setUsage(MTL::TextureUsageShaderRead);

  MTL::Texture* texture = device->newTexture(texDesc);
  
  // Copy pixel data to the Metal texture
  MTL::Region region = MTL::Region::Make2D(0, 0, width, height);
  texture->replaceRegion(region, 0, pixels.data(), width * 4);

  // NOTE: If you call this here, we get a seg fault.
  // texDesc->release();

  // Cleanup.
  CFRelease(pathCF);
  CFRelease(url);
  CFRelease(imgSource);
  CGContextRelease(context);
  CGImageRelease(image);

  return texture;
}


}  // namespace pixelengine::storage