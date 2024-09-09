//
// Created by Nathaniel Rupprecht on 9/6/24.
//

// Pixel engine.
#include <pixelengine/Utility.h>
#include <pixelengine/application/Game.h>
#include <pixelengine/input/Input.h>

using namespace pixelengine;
using namespace pixelengine::world;

// Some sand colors.
// 284 127 125
// 204 171 114
// 200 168 113
// 179 149 100




int main(int argc, char* argv[]) {

  app::Game game(512, 512, 512, 512);

  game.Initialize();
  game.Run();
  game.Finalize();

  // // Initialize the input object.
  // pixelengine::input::Input::Initialize();
  //
  // // Set up.
  // auto&& world = game.GetWorld();
  //
  // FallingPhysics falling{};
  // LiquidPhysics liquid{};
  //
  // for (auto j = 0u; j < world.GetHeight(); ++j) {
  //   for (auto i = 0u; i < world.GetWidth(); ++i) {
  //     auto r = randf();
  //     if (r < 0.2) {
  //       if (i < world.GetWidth() / 2) {
  //         auto c = randf();
  //         Square sand_square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
  //         world.SetSquare(i, j, sand_square);
  //       }
  //       else {
  //         Square water_square(true, Color::FromFloats(0., 0., 1.), &WATER, &liquid);
  //         world.SetSquare(i, j, water_square);
  //       }
  //     }
  //     else {
  //       world.SetSquare(i, j, Square(false, BACKGROUND, &AIR, nullptr));
  //     }
  //   }
  // }

  // End set up.



  return 0;
}



