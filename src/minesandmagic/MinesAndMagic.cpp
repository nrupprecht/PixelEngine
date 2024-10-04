#include "minesandmagic/MinesAndMagic.h"
// Other files.
#include "pixelengine/input/Input.h"
#include "pixelengine/world/World.h"

using namespace pixelengine;

using pixelengine::world::SAND;
using pixelengine::world::DIRT;
using pixelengine::world::WATER;
using pixelengine::world::Square;

namespace minesandmagic {


// TODO: Move these.
world::FallingPhysics falling {};
world::LiquidPhysics liquid {};
world::Stationary stationary {};

constexpr Color SAND_COLORS[] = {
  Color(204, 171, 114),
  Color(200, 168, 113),
  Color(179, 149, 100),
  Color(179, 149, 100),
};

// A background color.
constexpr Color BACKGROUND = Color(240, 228, 228);


void MinesAndMagic::setup() {
  using namespace pixelengine::world;

  auto& world = getWorld();

  for (auto j = 0u; j < world.GetHeight(); ++j) {
    for (auto i = 0u; i < world.GetWidth(); ++i) {
      auto r = randf();
      if (r < 0.2) {
        // if (i < world.GetWidth() / 2) {
        auto c = randf();
        Square sand_square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
        world.SetSquare(i, j, sand_square);
      }
      else {
        world.SetSquare(i, j, Square(false, BACKGROUND, &AIR, nullptr));
      }
    }
  }
}

void MinesAndMagic::updateWorld(float delta) {
  auto& world = getWorld();

  static unsigned brush_type = 0;
  if (input::Input::IsJustPressed('D') /* D */) {
    brush_type += 1;
    brush_type = brush_type % 3;
  }

  // Update the world based on input.

  if (input::Input::IsLeftMousePressed()) {
    if (auto opt = getCursorPosition()) {
      auto [x, y] = *opt;

      // Generate randomly in a circle
      int radius = 5;
      for (int i = -radius; i < radius; ++i) {
        for (int j = -radius; j < radius; ++j) {
          if (i * i + j * j < radius * radius && 0 <= x + i && x + i < world.GetWidth() && 0 <= y + j
              && y + j < world.GetHeight())
          {
            if (world.GetSquare(x + i, y + j).is_occupied) {
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
                square =
                    Square(true, Color(randi(30, 60), randi(30, 60), randi(30, 60)), &DIRT, &stationary);
              }
              world.SetSquare(x + i, y + j, square);
            }
          }
        }
      }
    }
  }
}

}  // namespace pixelengine::minesandmagic