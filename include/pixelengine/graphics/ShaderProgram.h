//
// Created by Nathaniel Rupprecht on 9/21/24.
//

#pragma once

#include <string>
#include <Metal/Metal.hpp>

namespace pixelengine::graphics {


//! \brief Represents a shader program.
//!
//! For now, keeping it real simple.
class ShaderProgram {
public:
  explicit ShaderProgram(std::string body,
                         std::string vertex_function_name,
                         std::string fragment_function_name)
      : body_(std::move(body))
      , vertex_function_name_(std::move(vertex_function_name))
      , fragment_function_name_(std::move(fragment_function_name)) {}

  [[nodiscard]] const std::string& GetBody() const { return body_; }

  [[nodiscard]] const std::string& GetVertexFunctionName() const { return vertex_function_name_; }

  [[nodiscard]] const std::string& GetFragmentFunctionName() const { return fragment_function_name_; }

private:
  std::string body_;

  std::string vertex_function_name_, fragment_function_name_;
};

}  // namespace pixelengine::graphics
