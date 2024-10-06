//
// Created by Nathaniel Rupprecht on 9/22/24.
//


#include "pixelengine/graphics/ShaderProgram.h"
// Other files.
#include <Lightning/Lightning.h>

namespace pixelengine::graphics {

ShaderProgram::ShaderProgram(MTL::Device* device,
                             std::string_view body,
                             std::string_view vertex_function_name,
                             std::string_view fragment_function_name)
    : body_(body)
    , vertex_function_name_(vertex_function_name)
    , fragment_function_name_(fragment_function_name)
    , device_(device) {
  NS::Error* error = nullptr;
  MTL::Library* library =
      device->newLibrary(NS::String::string(body_.c_str(), NS::UTF8StringEncoding), nullptr, &error);
  LL_ASSERT(library, "could not create library: " << error->localizedDescription()->utf8String());

  MTL::RenderPipelineDescriptor* pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();

  // === Set blending ===
  auto* color_attachment = pipeline_descriptor->colorAttachments()->object(0);
  color_attachment->setBlendingEnabled(true);
  color_attachment->setRgbBlendOperation(MTL::BlendOperationAdd);
  color_attachment->setAlphaBlendOperation(MTL::BlendOperationAdd);
  color_attachment->setSourceRGBBlendFactor(MTL::BlendFactorSourceColor);
  color_attachment->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
  color_attachment->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceColor);
  color_attachment->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
  // === End blending ===

  if (!vertex_function_name_.empty()) {
    auto vertex_function = createFunction(vertex_function_name_, library);
    pipeline_descriptor->setVertexFunction(vertex_function);
    vertex_function->release();
  }

  if (!fragment_function_name_.empty()) {
    auto fragment_function = createFunction(fragment_function_name_, library);
    pipeline_descriptor->setFragmentFunction(fragment_function);
    fragment_function->release();
  }

  pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

  pipeline_state_ = device->newRenderPipelineState(pipeline_descriptor, &error);
  LL_ASSERT(pipeline_state_,
            "could not create pipeline state: " << error->localizedDescription()->utf8String());
  pipeline_descriptor->release();
  library->release();
}

void ShaderProgram::SetPipelineState(MTL::RenderCommandEncoder* cmd_encoder) const {
  cmd_encoder->setRenderPipelineState(pipeline_state_);
}

MTL::Function* ShaderProgram::createFunction(const std::string& name, MTL::Library* library) {
  return library->newFunction(NS::String::string(name.c_str(), NS::UTF8StringEncoding));
}

}  // namespace pixelengine::graphics