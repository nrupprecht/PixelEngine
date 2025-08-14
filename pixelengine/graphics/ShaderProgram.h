//
// Created by Nathaniel Rupprecht on 9/21/24.
//

#pragma once

#include <string>

#include <Metal/Metal.hpp>

#include "pixelengine/utility/Utility.h"

namespace pixelengine::graphics {

namespace shadertypes {

//! \brief A vertex data structure used in some shader programs.
struct VertexData {
  simd::float3 position;
  simd::float2 texcoord;
};

}  // namespace shadertypes


//! \brief Represents a shader program.
//!
//! For now, keeping it real simple.
class ShaderProgram {
public:
  explicit ShaderProgram(MTL::Device* device,
                         std::string_view body,
                         std::string_view vertex_function_name,
                         std::string_view fragment_function_name);

  void SetPipelineState(MTL::RenderCommandEncoder* cmd_encoder) const;

  [[nodiscard]] MTL::Device* GetDevice() const { return device_; }

private:
  static MTL::Function* createFunction(const std::string& name, MTL::Library* library);

  std::string body_;

  std::string vertex_function_name_, fragment_function_name_;

  MTL::RenderPipelineState* pipeline_state_ {};

  MTL::Device* device_ {};
};

}  // namespace pixelengine::graphics
