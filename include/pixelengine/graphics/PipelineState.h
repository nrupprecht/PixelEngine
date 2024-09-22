//
// Created by Nathaniel Rupprecht on 9/21/24.
//

#pragma once

#include "pixelengine/graphics/ShaderProgram.h"

namespace pixelengine::graphics {


//! \brief Class for working with the metal pipeline state.
class PipelineState {
public:
  PipelineState(MTL::Device* device, const ShaderProgram& program);

  ~PipelineState();

  void SetPipelineState(MTL::RenderCommandEncoder* cmd_encoder) const;

  [[nodiscard]] MTL::RenderPipelineState* GetState() const { return pipeline_state_; }

private:
  static MTL::Function* createFunction(const std::string& name, MTL::Library* library);

  MTL::RenderPipelineState* pipeline_state_;
};

}  // namespace pixelengine::graphics
