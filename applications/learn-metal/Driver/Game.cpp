//
// Created by Nathaniel Rupprecht on 9/6/24.
//

// Pixel engine.
#include <pixelengine/application/Game.h>

#include "minesandmagic/MinesAndMagic.h"

using namespace pixelengine;
using namespace pixelengine::world;


int main(int argc, char* argv[]) {

  lightning::Global::GetCore()->AddSink(lightning::NewSink<lightning::StdoutSink>());

  // Game window dimensions.
  Dimensions window_dimensions {1400, 800};

  minesandmagic::MinesAndMagic game(432, 240, 432, 240, window_dimensions);

  game.Initialize();
  game.Run();

  return 0;
}
