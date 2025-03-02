#include "minesandmagic/MinesAndMagic.h"
// Other files.
#include "pixelengine/graphics/ShaderStore.h"
#include "pixelengine/input/Input.h"
#include "pixelengine/utility/Contracts.h"

#include "minesandmagic/Materials.h"
#include "minesandmagic/Player.h"
#include "minesandmagic/SingleChunkWorld.h"


using namespace pixelengine;

namespace minesandmagic {

void MinesAndMagic::setup() {
  using namespace pixelengine::world;

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

  auto player = std::make_unique<Player>(PVec2 {.x = 50, .y = 180}, 8, 16);
  player->SetName("Player");


  auto program = graphics::ShaderStore::GetInstance()->GetShaderProgram("TextureShader");

  auto texture = std::make_unique<graphics::TextureBitmapOwning>(12, 16, program->GetDevice());
  auto& bitmap = texture->GetTextureBitmap();
  bitmap.SetAllPixels(Color(255, 0, 0, 255));
  auto sprite = std::make_unique<graphics::RectangularDrawable>(program, 12, 16, std::move(texture));

  // auto sprite = graphics::LoadBMP("/Users/nathaniel/Documents/Nathaniel/Programs/C++/PixelEngine/assets/Little-Sprite.bmp");
  PIXEL_ASSERT(sprite, "could not load player sprite");
  sprite->SetName("PlayerSprite");

  // Give the sprite a texture.
  player->AddChild(std::move(sprite));

  world->AddChild(std::move(player));

  addNode(std::move(world));
}

}  // namespace minesandmagic