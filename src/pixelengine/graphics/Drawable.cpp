//
// Created by Nathaniel Rupprecht on 9/21/24.
//

#include "pixelengine/graphics/Drawable.h"

namespace pixelengine::graphics {

Drawable::Drawable(std::unique_ptr<PipelineState> pipeline_state)
      : pipeline_state_(std::move(pipeline_state)) {}

void Drawable::Draw(MTL::RenderCommandEncoder* cmd_encoder) {
  // Set pipeline state.
  pipeline_state_->SetPipelineState(cmd_encoder);

  updateTextures();
  setArguments(cmd_encoder);
  draw(cmd_encoder);
}

void Drawable::updateTextures() {
  std::ranges::for_each(textures_, [](auto& texture) { texture->Update(); });
}

void Drawable::setArguments(MTL::RenderCommandEncoder* cmd_encoder) {
  for (auto idx = 0; idx < buffers_.size(); ++idx) {
    cmd_encoder->setVertexBuffer(buffers_[idx].Data(), 0, idx);
  }
  for (auto idx = 0; idx < fragment_buffers_.size(); ++idx) {
    cmd_encoder->setFragmentBuffer(fragment_buffers_[idx].Data(), 0, idx);
  }
  for (auto idx = 0; idx < textures_.size(); ++idx) {
    cmd_encoder->setFragmentTexture(textures_[idx]->GetTexture(), idx);
  }
}

}  // namespace pixelengine::graphics