//
// Created by Nathaniel Rupprecht on 10/4/24.
//

#pragma once

#include <optional>
#include <string_view>
#include <unordered_map>
#include <deque>

#include "pixelengine/graphics/ShaderProgram.h"

namespace pixelengine::app {
class Game;
}  // namespace app

namespace pixelengine::graphics {



class ShaderStore {
  friend class ::pixelengine::app::Game;

public:
  static ShaderStore* GetInstance();

  [[nodiscard]] ShaderProgram* GetShaderProgram(const std::string& name) const;

  ShaderProgram* CreateShaderProgram(const std::string& name,
                                     std::string_view body,
                                     std::string_view vertex_function_name,
                                     std::string_view fragment_function_name);

private:
  ShaderStore(MTL::Device* device) : device_(device) {}

  static void makeGlobalInstance(MTL::Device* device);
  static void cleanGlobalInstance();

  void compileUncompiledShaders();

  MTL::Device* device_ {};
  std::deque<std::unique_ptr<ShaderProgram>> shader_programs_;
  std::unordered_map<std::string, ShaderProgram*> access_;
};

}  // namespace pixelengine::graphics