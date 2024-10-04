//
// Created by Nathaniel Rupprecht on 9/28/24.
//

#pragma once

#include "pixelengine/application/Game.h"

namespace minesandmagic {

class MinesAndMagic : public pixelengine::app::Game {

public:
  MinesAndMagic(int tile_width, int tile_height, int window_width, int window_height, const pixelengine::Dimensions& window_dimensions)
    : Game(tile_width, tile_height, window_width, window_height, window_dimensions) {}

private:
  void setup() override;

  void updateWorld(float delta) override;
};

}  // namespace pixelengine::minesandmagic