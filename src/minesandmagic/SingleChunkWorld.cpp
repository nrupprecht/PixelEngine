//
// Created by Nathaniel Rupprecht on 10/4/24.
//

#include "minesandmagic/SingleChunkWorld.h"
// Other files.
#include <pixelengine/graphics/ShaderStore.h>
#include <pixelengine/input/Input.h>
#include <pixelengine/utility/Contracts.h>

#include "minesandmagic/Materials.h"

using namespace pixelengine;

namespace minesandmagic {

SingleChunkWorld::SingleChunkWorld(std::size_t chunk_width, std::size_t chunk_height)
    : chunk_width_(chunk_width)
    , chunk_height_(chunk_height)
    , active_region_(0, static_cast<long long>(chunk_width), 0, static_cast<long long>(chunk_height))
    , squares_(chunk_width_ * chunk_height_) {
  std::string shader = R"(
      #include <metal_stdlib>
      using namespace metal;

      struct VertexData
       {
           float3 position;
           float2 texcoord;
       };

       struct VertexFragment
       {
         float4 position [[position]];
         float2 texcoord;
       };

      VertexFragment vertex vertexMain( device const VertexData* vertexData [[buffer(0)]], uint vertexID [[vertex_id]] )
      {
        VertexFragment o;
        o.position = float4( vertexData[ vertexID ].position, 1.0 );
        o.texcoord = vertexData[ vertexID ].texcoord;
        return o;
      }

      half4 fragment fragmentMain(
        VertexFragment in [[stage_in]],
        texture2d<half, access::sample> tex [[texture(0)]]
      )
      {
        constexpr float scale = 0.01;

        constexpr sampler s( address::repeat, filter::nearest );
        half3 texel = tex.sample( s, in.texcoord ).rgb;
        return half4( texel, 1.0 );
      }
  )";

  auto shader_program = graphics::ShaderStore::GetInstance()->CreateShaderProgram(
      "WorldShader", shader, "vertexMain", "fragmentMain");
  PIXEL_ASSERT(shader_program, "could not create shader program");

  world_texture_.Initialize(chunk_width, chunk_height, shader_program->GetDevice());

  auto drawable = std::make_unique<graphics::RectangularDrawable>(
      shader_program, std::make_unique<graphics::TextureWrapper>(world_texture_.GetTexture()));

  AddChild(std::move(drawable));
}


void SingleChunkWorld::_draw(MTL::RenderCommandEncoder* render_command_encoder,
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

void SingleChunkWorld::_updatePhysics(float raw_dt) {
  auto dt = std::min(1.f / 30.f, raw_dt);

  // auto begin_evolution_time = std::chrono::high_resolution_clock::now();

  // Reset was-moved flags.
  for (auto y = 0; y < chunk_height_; ++y) {
    for (auto x = 0; x < chunk_width_; ++x) {
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

    square.UpdateKinematics(dt, gravity_);
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

  // TODO: Other updates, e.g. temperature, objects catching fire, reacting, etc.

  // auto end_evoluation_time = std::chrono::high_resolution_clock::now();

  // Get the time it took to evolve.
  // auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_evoluation_time -
  // begin_evolution_time);
  //
  // auto evolution_time = static_cast<float>(duration.count()) / 1'000'000.f;
  // std::cout << "dt = " << evolution_time << ", FPS = " << 1. / evolution_time << ", BB = " <<
  // active_region_ << std::endl;
}

void SingleChunkWorld::_update(float dt) {
  static unsigned brush_type = 0;
  if (input::Input::IsJustPressed('D') /* D */) {
    brush_type += 1;
    brush_type = brush_type % 3;
  }

  // Update the world based on input.

  if (input::Input::IsLeftMousePressed()) {
    if (auto opt = input::Input::GetApplicationCursorPosition()) {
      auto [fx, fy] = *opt;

      auto x = static_cast<long long>(fx * chunk_width_);
      auto y = static_cast<long long>(fy * chunk_height_);

      // Generate randomly in a circle
      int radius = 5;
      for (int i = -radius; i < radius; ++i) {
        for (int j = -radius; j < radius; ++j) {
          if (i * i + j * j < radius * radius && 0 <= x + i && x + i < GetWidth() && 0 <= y + j
              && y + j < GetHeight())
          {
            if (GetSquare(x + i, y + j).is_occupied) {
              continue;
            }
            if (randf() < 0.7) {
              auto c = randf();

              Square square;

              if (brush_type == 0) {
                square            = Square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
                square.velocity_y = -50;
              }
              else if (brush_type == 1) {
                square            = Square(true, Color::FromFloats(0., 0., 1.), &WATER, &liquid);
                square.velocity_y = -50;
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