//
// Created by Nathaniel Rupprecht on 10/4/24.
//

#pragma once

#include "pixelengine/graphics/RectangularDrawable.h"
#include "pixelengine/world/World.h"

namespace minesandmagic {

using namespace pixelengine::world;
using namespace pixelengine::application;

//! \brief The physical game world.
class SingleChunkWorld : public World {
public:
  SingleChunkWorld(std::size_t chunk_width, std::size_t chunk_height);

  [[nodiscard]] std::size_t GetWidth() const { return chunk_width_; }
  [[nodiscard]] std::size_t GetHeight() const { return chunk_height_; }
  [[nodiscard]] float GetGravity() const override { return gravity_; }

  [[nodiscard]] const BoundingBox& GetActiveRegion() const { return active_region_; }

private:
  void _update(float dt) override;

  void _updatePhysics(float dt, const World* world) override;

  void _draw(MTL::RenderCommandEncoder* render_command_encoder,
             WindowContext* context,
             pixelengine::Vec2 parent_offset) override;

  WindowContext* _chooseWindowContext([[maybe_unused]] WindowContext* context) override {
    return &world_context_; // What if the incoming context is not null?
  }

  void setSquare(long long x, long long y, const Square& square) override {
    active_region_.Update(x, y);
    getSquare(x, y) = square;
  }

  [[nodiscard]] bool isValidSquare(long long x, long long y) const override {
    return 0 <= x && x < static_cast<long long>(chunk_width_) && 0 <= y && y < static_cast<long long>(chunk_height_);
  }

  std::size_t chunk_width_;
  std::size_t chunk_height_;

  WindowContext world_context_;

  BoundingBox active_region_;

  //! \brief Acceleration due to gravity, in squares per second squared.
  float gravity_ = -100.;

  mutable pixelengine::TextureBitmap world_texture_;
  std::shared_ptr<pixelengine::graphics::RectangularDrawable> main_drawable_;

  Square& getSquare(long long x, long long y) override {
    LL_ASSERT(x < static_cast<long long>(chunk_width_) && y < static_cast<long long>(chunk_height_), "out of bounds, x, y = " << x << ", " << y);
    return squares_[y * chunk_width_ + x];
  }

  [[nodiscard]] const Square& getSquare(long long x, long long y) const override {
    LL_ASSERT(0 <= x && x < static_cast<long long>(chunk_width_) && 0 <= y && y < static_cast<long long>(chunk_height_),
              "out of bounds, x, y = " << x << ", " << y);
    return squares_[y * chunk_width_ + x];
  }

  std::vector<Square> squares_;
};


}  // namespace minesandmagic