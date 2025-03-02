//
// Created by Nathaniel Rupprecht on 10/29/24.
//

#pragma once

#include "pixelengine/world/World.h"

namespace minesandmagic {

using pixelengine::world::Square;

class ChunkedWorld : public pixelengine::world::World {
private:
  [[nodiscard]] const Square& getSquare(long long x, long long y) const override;
  [[nodiscard]] Square& getSquare(long long x, long long y) override;
  void setSquare(long long x, long long y, const Square& square) override;
  [[nodiscard]] bool isValidSquare(long long x, long long y) const override;

  int32_t chunk_width_{256};
  int32_t chunk_height_{256};

  int32_t chunks_to_cache_; // Number of chunks to cache in memory
};

}  // namespace minesandmagic