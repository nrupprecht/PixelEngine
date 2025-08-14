//
// Created by Nathaniel Rupprecht on 10/4/24.
//

#include "minesandmagic/SingleChunkWorld.h"
// Other files.
#include "minesandmagic/Materials.h"
#include "pixelengine/graphics/ShaderStore.h"
#include "pixelengine/input/Input.h"
#include "pixelengine/utility/Contracts.h"

using namespace pixelengine;

namespace minesandmagic {

SingleChunkWorld::SingleChunkWorld(std::size_t chunk_width, std::size_t chunk_height)
    : chunk_width_(chunk_width)
    , chunk_height_(chunk_height)
    // TODO: Get the actual window width and height? Do we really need these in this struct?
    , world_context_ {0.f, static_cast<float>(chunk_width), 0.f, static_cast<float>(chunk_height)}
    , active_region_(0, static_cast<long long>(chunk_width), 0, static_cast<long long>(chunk_height))
    , squares_(chunk_width_ * chunk_height_) {
  auto shader_program = graphics::ShaderStore::GetInstance()->GetShaderProgram("TextureShader");
  PIXEL_ASSERT(shader_program, "could not get shader program");

  // The world texture is a bitmap that we draw the "sand" (and other material) chunks to.
  // Each square in the world is represented by a pixel in the texture.
  // The texture is chunk_width_ pixels wide and chunk_height_ pixels tall.
  // The size of the texture does not have to match the actual resolution / size of the window.
  world_texture_.Initialize(chunk_width, chunk_height, shader_program->GetDevice());

  auto drawable = std::make_unique<graphics::RectangularDrawable>(
      shader_program,
      chunk_width,
      chunk_height,
      std::make_unique<graphics::TextureWrapper>(world_texture_.GetTexture()));
  drawable->SetName("WorldTexture");

  AddChild(std::move(drawable));
}


void SingleChunkWorld::_draw(MTL::RenderCommandEncoder* render_command_encoder,
                             [[maybe_unused]] WindowContext* context,
                             pixelengine::Vec2 parent_offset) {
  // Update pixels to render the world.
  for (auto j = 0ull; j < world_texture_.GetHeight(); ++j) {
    for (auto i = 0ull; i < world_texture_.GetWidth(); ++i) {
      auto x        = i;
      auto y        = world_texture_.GetHeight() - 1 - j;
      auto&& square = GetSquare(i, j);

      world_texture_.SetPixel(x, y, square.color);
    }
  }
  // Update the metal texture behind the texture bitmap.
  world_texture_.Update();
}

void SingleChunkWorld::_updatePhysics(float raw_dt, const world::World* world) {
  auto dt = std::min(1.f / 30.f, raw_dt);

  // Reset was-moved flags.
  for (auto y = 0; y < static_cast<long long>(chunk_height_); ++y) {
    for (auto x = 0; x < static_cast<long long>(chunk_width_); ++x) {
      auto&& square    = getSquare(x, y);
      square.num_moves = 0;
    }
  }

  // Get the region in which updates need to occur.
  active_region_.Expand(1);
  auto [x_min, x_max, y_min, y_max] =
      active_region_.Clip(static_cast<long long>(chunk_width_), static_cast<long long>(chunk_height_));

  // BB for determining the next active zone.
  BoundingBox bounding_box;

  auto update = [this, dt, &bounding_box](long long x, long long y) {
    auto&& square = getSquare(x, y);
    if (!square.is_occupied || square.material->is_rigid || !square.behavior /* || 0 < square.num_moves*/) {
      return;
    }

    square.UpdateKinematics(dt, *this);
    if (auto bb = square.behavior->Update(dt, x, y, *this); !bb.IsEmpty()) {
      bounding_box.Update(bb);
    }
  };

  // Update motion.
  for (auto y = y_min; y <= y_max; ++y) {
    if (pixelengine::randf() < 0.5) {
      for (auto x = x_min; x <= x_max; ++x) {
        update(x, y);
      }
    }
    else {
      for (auto x = x_max; x >= x_min; --x) {
        update(x, y);
      }
    }
  }

  // Update active region.
  active_region_ = bounding_box;

  // TODO: Other updates, e.g. temperature, objects catching fire, reacting, etc.?
}

void SingleChunkWorld::_update(float dt) {
  static unsigned brush_type = 0;
  // TODO: Use input callbacks instead?
  if (input::Input::IsJustPressed('B')) {
    brush_type += 1;
    brush_type = brush_type % 3;
    LOG_SEV(Debug) << "Changed brush type to " << brush_type;
  }

  // Update the world based on input.

  if (input::Input::IsLeftMousePressed()) {
    if (auto opt = input::Input::GetApplicationCursorPosition()) {
      auto [fx, fy] = *opt;

      auto x = static_cast<long long>(fx * chunk_width_);
      auto y = static_cast<long long>(fy * chunk_height_);

      // Generate randomly in a circle
      int radius = 10;
      float p    = 0.7;  // 0.7
      for (int i = -radius; i <= radius; ++i) {
        for (int j = -radius; j <= radius; ++j) {
          if (i * i + j * j <= radius * radius) {
            if (!IsValidSquare(x + i, y + j)) {
              continue;
            }
            if (GetSquare(x + i, y + j).is_occupied) {
              continue;
            }
            if (randf() < p) {
              auto c = randf();

              Square square;

              if (brush_type == 0) {
                square            = Square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
                square.velocity.y = -50;
              }
              else if (brush_type == 1) {
                square            = Square(true, Color::FromFloats(0., 0., 1.), &WATER, &liquid);
                square.velocity.y = -50;
              }
              else if (brush_type == 2) {
                square = Square(true, Color(randi(30, 60), randi(30, 60), randi(30, 60)), &DIRT, &stationary);
              }
              SetSquare(x + i, y + j, square);
            }
          }
        }
      }
    }
  }
}


}  // namespace minesandmagic