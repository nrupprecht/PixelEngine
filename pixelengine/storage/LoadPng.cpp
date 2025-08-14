

#include "pixelengine/storage/LoadPng.h"

namespace pixelengine::storage {

MTL::Texture* LoadMTLTextureFromPNG(MTL::Device* device, std::string_view path) {
    // Create URL for file
    const CFStringRef cfPath = CFStringCreateWithCString(nullptr, path.data(), kCFStringEncodingUTF8);
    const CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, cfPath, kCFURLPOSIXPathStyle, false);

    // Load image
    const CGImageSourceRef src = CGImageSourceCreateWithURL(url, nullptr);
    const CGImageRef img = CGImageSourceCreateImageAtIndex(src, 0, nullptr);

    const std::size_t width = CGImageGetWidth(img);
    const std::size_t height = CGImageGetHeight(img);

    std::vector<uint8_t> pixelData(width * height * 4);
    CGContextRef context = CGBitmapContextCreate(pixelData.data(),
        width, height, 8, width * 4, CGImageGetColorSpace(img),
        kCGImageAlphaPremultipliedLast);

    CGContextDrawImage(context, CGRectMake(0, 0, width, height), img);

    // Create Metal texture descriptor
    MTL::TextureDescriptor* texDesc =
        MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatRGBA8Unorm,
                                                    width, height, false);
    texDesc->setUsage(MTL::TextureUsageShaderRead);

    MTL::Texture* texture = device->newTexture(texDesc);

    // Copy pixel data into Metal texture
    MTL::Region region = MTL::Region::Make2D(0, 0, width, height);
    texture->replaceRegion(region, 0, pixelData.data(), width * 4);

    // Cleanup
    CFRelease(cfPath);
    CFRelease(url);
    CFRelease(src);
    CGImageRelease(img);
    CGContextRelease(context);

    return texture;
}

}  // namespace pixelengine::storage