//
// Created by Nathaniel Rupprecht on 10/4/24.
//


#include "pixelengine/graphics/ShaderStore.h"
// Other files.
#include <Lightning/Lightning.h>

namespace pixelengine::graphics {

namespace {

ShaderStore* instance_ = nullptr;

void createBasicShaders() {
  // Create the TextureShader, so it can be used by other things.
  std::string shader = R"(
      #include <metal_stdlib>
      using namespace metal;

      struct VertexData {
        float3 position;
        float2 texcoord;
      };

      struct VertexFragment {
        float4 position [[position]];
        float2 texcoord;
      };

      VertexFragment vertex vertexMain( device const VertexData* vertexData [[buffer(0)]], uint vertexID [[vertex_id]]) {
        VertexFragment o;
        o.position = float4(vertexData[vertexID].position, 1.0 );
        o.texcoord = vertexData[vertexID].texcoord;
        return o;
      }

      half4 fragment fragmentMain(VertexFragment in [[stage_in]], texture2d<half, access::sample> tex [[texture(0)]]) {
        constexpr sampler s( address::repeat, filter::nearest );
        half3 texel = tex.sample( s, in.texcoord ).rgb;
        return half4( texel, 1.0 );
      }
  )";

  instance_->CreateShaderProgram("TextureShader", shader, "vertexMain", "fragmentMain");
}

}  // namespace

ShaderStore* ShaderStore::GetInstance() {
  if (!instance_) {
    LOG_SEV(Warning) << "Cannot create a ShaderProgram, ShaderStore instance is not initialized.";
  }
  return instance_;
}

ShaderProgram* ShaderStore::GetShaderProgram(const std::string& name) const {
  if (auto it = access_.find(name); it != access_.end()) {
    return it->second;
  }
  return {};
}

ShaderProgram* ShaderStore::CreateShaderProgram(const std::string& name,
                                                std::string_view body,
                                                std::string_view vertex_function_name,
                                                std::string_view fragment_function_name) {
  auto it = access_.find(name);
  if (it != access_.end()) {
    return {};
  }
  auto program = std::make_unique<ShaderProgram>(device_, body, vertex_function_name, fragment_function_name);
  shader_programs_.emplace_back(std::move(program));
  auto ptr = shader_programs_.back().get();
  access_.emplace(name, ptr);
  return ptr;
}

void ShaderStore::makeGlobalInstance(MTL::Device* device) {
  instance_ = new ShaderStore(device);

  // Create some basic shaders
  createBasicShaders();
}

void ShaderStore::cleanGlobalInstance() {
  delete instance_;
  instance_ = nullptr;
}

}  // namespace pixelengine::graphics
