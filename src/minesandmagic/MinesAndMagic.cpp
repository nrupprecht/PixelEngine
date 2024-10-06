#include "minesandmagic/MinesAndMagic.h"
// Other files.
#include <pixelengine/input/Input.h>

#include "minesandmagic/Materials.h"
#include "minesandmagic/Player.h"
#include "minesandmagic/SingleChunkWorld.h"

using namespace pixelengine;

namespace minesandmagic {

void MinesAndMagic::setup() {
  using namespace pixelengine::world;

  auto world = std::make_unique<SingleChunkWorld>(texture_width_, texture_height_);

  for (auto j = 0u; j < world->GetHeight(); ++j) {
    for (auto i = 0u; i < world->GetWidth(); ++i) {
      auto r = randf();
      if (r < 0.2) {
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


  auto player = std::make_unique<Player>(Vec2 {0.5, 0.5}, 1., 1.);
  // TODO: Give the player a sprite.
  // world->AddChild(std::move(player));

  addNode(std::move(world));
}

}  // namespace minesandmagic