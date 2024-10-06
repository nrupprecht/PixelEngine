//
// Created by Nathaniel Rupprecht on 9/28/24.
//

#pragma once

#include "pixelengine/application/Game.h"

namespace minesandmagic {

class MinesAndMagic : public pixelengine::app::Game {
public:
  MinesAndMagic(uint32_t texture_width,
                uint32_t texture_height,
                int window_width,
                int window_height,
                const pixelengine::Dimensions& window_dimensions)
      : Game(window_width, window_height, window_dimensions)
      , texture_width_(texture_width)
      , texture_height_(texture_height) {}

private:
  void setup() override;

  uint32_t texture_width_;
  uint32_t texture_height_;
};

}  // namespace minesandmagic