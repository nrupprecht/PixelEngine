//
// Created by Nathaniel Rupprecht on 9/9/24.
//

#pragma once

#include <thread>
#include <list>
#include "pixelengine/application/AppDelegate.h"
#include "pixelengine/world/World.h"
#include "pixelengine/graphics/RectangularDrawable.h"

#include "pixelengine/Node.h"

namespace pixelengine::app {

//! \brief Class for the game engine.
class Game {
public:
  Game(uint32_t texture_width, uint32_t texture_height, std::size_t width, std::size_t height, Dimensions window_dimensions)
    : texture_width_(texture_width)
    , texture_height_(texture_height)
    , world_(std::make_unique<world::World>(width, height))
    , window_dimensions_(window_dimensions) {}

  ~Game() = default;

  //! \brief Do any setup before running the game.
  void Initialize();

  //! \brief Run the game loop.
  void Run();

  void SetFrame(CGRect window_frame) { window_frame_ = window_frame; }
  [[nodiscard]] CGRect GetFrame() const { return window_frame_; }

  //! \brief Set up the texture.
  // void InitializeTexture(TextureBitmap& texture_bitmap, MTL::Device* device) const;

  //! \brief Get the world that the game manages.
  [[nodiscard]] world::World& GetWorld() const { return *world_; }

protected:
  //! \brief Load resources.
  virtual void initialize() {}

  //! \brief Set up the game world.
  virtual void setup() {}

  virtual void updateWorld(float delta) {}

  virtual void finalize() {}

  [[nodiscard]] world::World& getWorld() { return *world_; }

  std::optional<std::array<long long, 2>> getCursorPosition() const;
private:
  void setDelegates();

  //! \brief Update step. Calls all the other update functions.
  void update(float delta);

  void updateWorldPhysics(float delta);

  //! \brief Draw the visible world to a texture.
  void draw(TextureBitmap& texture_bitmap) const;

  void drawTexture() const;

  static void simulation(Game* game);

  // ===========================================================================
  // Private data.
  // ===========================================================================

  const uint32_t texture_width_;
  const uint32_t texture_height_;

  bool run_simulation_independently_ = false;

  Dimensions window_dimensions_;

  std::mutex world_mutex_{};

  //! \brief Pointer to the world.
  std::unique_ptr<world::World> world_{};

  CGRect window_frame_{};

  std::unique_ptr<GameAppDelegate> application_{};

  std::deque<std::unique_ptr<graphics::ShaderProgram>> shader_programs_;
  mutable TextureBitmap world_texture_;
  std::shared_ptr<graphics::RectangularDrawable> main_drawable_;

  std::thread simulation_thread_;

  volatile bool is_initialized_ = false;
  bool is_running_ = false;

  //! \brief All the top level nodes in the game.
  std::list<std::unique_ptr<Node>> nodes_;
};

}  // namespace pixelengine::app