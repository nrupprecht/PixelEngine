//
// Created by Nathaniel Rupprecht on 9/6/24.
//

// Pixel engine.
#include <pixelengine/application/Game.h>

using namespace pixelengine;
using namespace pixelengine::world;


int main(int argc, char* argv[]) {
  // Game window dimensions.
  Dimensions window_dimensions {1400, 800};

  app::Game game(432, 240, 432, 240, window_dimensions);

  game.Initialize();
  game.Run();
  game.Finalize();

  return 0;
}
