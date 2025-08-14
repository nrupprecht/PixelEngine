#include "minesandmagic/MinesAndMagic.h"
// Other files.
#include "minesandmagic/Materials.h"
#include "minesandmagic/Player.h"
#include "minesandmagic/SingleChunkWorld.h"
#include "pixelengine/graphics/ShaderStore.h"
#include "pixelengine/input/Input.h"
#include "pixelengine/storage/LoadImage.h"
#include "pixelengine/utility/Contracts.h"


using namespace pixelengine;

namespace minesandmagic {

void MinesAndMagic::setup() {
  using namespace pixelengine::world;

  auto world = std::make_unique<SingleChunkWorld>(texture_width_, texture_height_);
  world->SetName("World");

  // Setup the materials in the world.
  for (auto j = 0u; j < world->GetHeight() / 2; ++j) {
    for (auto i = 0u; i < world->GetWidth(); ++i) {
      auto r = randf();
      if (r < 0.8) {
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
  PIXEL_ASSERT(program, "could not get shader program");

  // auto texture = std::make_unique<graphics::TextureBitmapOwning>(12, 16, program->GetDevice());
  // auto& bitmap = texture->GetTextureBitmap();
  // bitmap.SetAllPixels(Color(0, 255, 0, 255));

  const char* map = "/Users/nrupprecht/Desktop/game/A_top-down,_2D_digital_illustration_depicts_a_fict.png";
  const char* little_sprite = "/Users/nrupprecht/Desktop/game/Little-Sprite.bmp";

  auto texture = pixelengine::storage::LoadTextureFromImage(program->GetDevice(), map);
  // auto texture = pixelengine::storage::LoadTextureFromImage(program->GetDevice(), little_sprite);


  // auto sprite = std::make_unique<graphics::RectangularDrawable>(program, 12, 16, std::move(texture));
  auto sprite = std::make_unique<graphics::RectangularDrawable>(program, 24, 24, std::move(texture));

  PIXEL_ASSERT(sprite, "could not load player sprite");
  sprite->SetName("PlayerSprite");

  // Give the sprite a texture.
  player->AddChild(std::move(sprite));

  world->AddChild(std::move(player));

  // Add the world as a child of the game.
  addNode(std::move(world));
}

}  // namespace minesandmagic