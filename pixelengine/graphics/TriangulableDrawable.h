//
// Created by Nathaniel Rupprecht on 9/22/24.
//

#pragma once

#include <array>

#include "pixelengine/graphics/Color.h"
#include "pixelengine/graphics/Drawable.h"
#include "pixelengine/graphics/ShaderProgram.h"

namespace pixelengine::graphics {

//! \brief A rectangular drawable object.
class TriangulableDrawable : public Drawable {
public:
//! \brief TODO: more general shader program support?
  TriangulableDrawable(std::vector<Vec2> vertices, std::vector<uint16_t> indices, Color color);

private:
  void drawVertices(MTL::RenderCommandEncoder* cmd_encoder,
                    application::WindowContext* context,
                    Vec2 parent_offset) override;

  utility::AutoBuffer index_buffer_;
  std::vector<Vec2> vertices_;
};

}  // namespace pixelengine::graphics