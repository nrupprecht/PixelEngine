#include "minesandmagic/MinesAndMagic.h"
// Other files.
#include <pixelengine/graphics/ShaderStore.h>
#include <pixelengine/input/Input.h>

#include "minesandmagic/Materials.h"
#include "minesandmagic/Player.h"
#include "minesandmagic/SingleChunkWorld.h"

using namespace pixelengine;

namespace minesandmagic {

void MinesAndMagic::setup() {
  using namespace pixelengine::world;

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

  graphics::ShaderStore::GetInstance()->CreateShaderProgram(
      "TextureShader", shader, "vertexMain", "fragmentMain");

  auto world = std::make_unique<SingleChunkWorld>(texture_width_, texture_height_);
  world->SetName("World");

  for (auto j = 0u; j < world->GetHeight() / 2; ++j) {
    for (auto i = 0u; i < world->GetWidth(); ++i) {
      auto r = randf();
      if (r < 0.8) {
        // if (i < world.GetWidth() / 2) {
        auto c = randf();
        Square sand_square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
        world->SetSquare(i, j, sand_square);
      }
      else {
        world->SetSquare(i, j, Square(false, BACKGROUND, &AIR, nullptr));
      }
    }
  }

  auto player = std::make_unique<Player>(PVec2 {50, 180}, 8, 16);
  player->SetName("Player");

  auto program = graphics::ShaderStore::GetInstance()->GetShaderProgram("TextureShader");

  auto texture = std::make_unique<graphics::TextureBitmapOwning>(12, 16, program->GetDevice());
  auto& bitmap = texture->GetTextureBitmap();
  bitmap.SetAllPixels(Color(255, 0, 0, 255));
  auto sprite = std::make_unique<graphics::RectangularDrawable>(program, std::move(texture));
  sprite->SetWidth(12.f);
  sprite->SetHeight(16.f);
  sprite->SetName("PlayerSprite");

  // Give the sprite a texture.
  player->AddChild(std::move(sprite));


  // TODO: Give the player a sprite.
  world->AddChild(std::move(player));

  addNode(std::move(world));
}

}  // namespace minesandmagic