//
// Created by Nathaniel Rupprecht on 9/21/24.
//

#include "pixelengine/graphics/PipelineState.h"
#include <Lightning/Lightning.h>

namespace pixelengine::graphics {

PipelineState::PipelineState(MTL::Device* device, const ShaderProgram& program) {
  auto&& shader = program.GetBody();

  NS::Error* error = nullptr;
  MTL::Library* library =
      device->newLibrary(NS::String::string(shader.c_str(), NS::UTF8StringEncoding), nullptr, &error);
  LL_ASSERT(library, "could not create library: " << error->localizedDescription()->utf8String());

  // Set that the vertex function is called `vertexMain`.
  auto vertex_function = createFunction(program.GetVertexFunctionName(), library);
  // Set that the fragment function is called `fragmentMain`.
  auto fragment_function = createFunction(program.GetFragmentFunctionName(), library);

  MTL::RenderPipelineDescriptor* pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
  pipeline_descriptor->setVertexFunction(vertex_function);
  pipeline_descriptor->setFragmentFunction(fragment_function);
  pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(
      MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

  pipeline_state_ = device->newRenderPipelineState(pipeline_descriptor, &error);
  LL_ASSERT(pipeline_state_,
            "could not create pipeline state: " << error->localizedDescription()->utf8String());

  vertex_function->release();
  fragment_function->release();
  pipeline_descriptor->release();
  library->release();
}

PipelineState::~PipelineState() {
  pipeline_state_->release();
}

void PipelineState::SetPipelineState(MTL::RenderCommandEncoder* cmd_encoder) const {
  cmd_encoder->setRenderPipelineState(pipeline_state_);
}


MTL::Function* PipelineState::createFunction(const std::string& name, MTL::Library* library) {
  return library->newFunction(NS::String::string(name.c_str(), NS::UTF8StringEncoding));
}

}  // namespace pixelengine::graphics