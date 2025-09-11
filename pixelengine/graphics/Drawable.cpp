//
// Created by Nathaniel Rupprecht on 9/21/24.
//

#include "pixelengine/graphics/Drawable.h"

namespace pixelengine::graphics {

Drawable::Drawable(ShaderProgram* shader_program) : shader_program_(shader_program) {}

void Drawable::_draw(MTL::RenderCommandEncoder* cmd_encoder) {
  // Set pipeline state - tells the device (GPU) to use the shader program (includes the 
  // vertex and fragment shaders).
  shader_program_->SetPipelineState(cmd_encoder);

  updateTextures();

  // Set the vertex / fragment buffers and textures in the command encoder that the shader program needs.
  setArguments(cmd_encoder);

  // Tell the child drawable to draw its vertices.
  // TODO: Is there a way to do all the translation of vertices generally here (we might have to store vertices here)?
  //       How much commonality is there between drawables? Do they all draw their vertices the same way?
  drawVertices(cmd_encoder);
}

void Drawable::updateTextures() {
  std::ranges::for_each(textures_, [](auto& texture) { texture->Update(); });
}

void Drawable::setArguments(MTL::RenderCommandEncoder* cmd_encoder) {
  for (std::size_t idx = 0; idx < buffers_.size(); ++idx) {
    cmd_encoder->setVertexBuffer(buffers_[idx].Data(), 0, idx);
  }
  for (std::size_t idx = 0; idx < fragment_buffers_.size(); ++idx) {
    cmd_encoder->setFragmentBuffer(fragment_buffers_[idx].Data(), 0, idx);
  }
  for (std::size_t idx = 0; idx < textures_.size(); ++idx) {
    cmd_encoder->setFragmentTexture(textures_[idx]->GetTexture(), idx);
  }
}

}  // namespace pixelengine::graphics