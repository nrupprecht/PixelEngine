//
// Created by Nathaniel Rupprecht on 9/9/24.
//

#pragma once

#include <thread>
#include <mutex>
#include "pixelengine/application/AppDelegate.h"
#include "pixelengine/world/World.h"

namespace pixelengine::app {

//! \brief Class for the game engine.
class Game {
public:
  Game(uint32_t texture_width, uint32_t texture_height, std::size_t width, std::size_t height)
    : texture_width_(texture_width)
    , texture_height_(texture_height)
    , world_(std::make_unique<world::World>(width, height))
  , application_(std::make_unique<GameAppDelegate>()) {}

  ~Game() = default;

  //! \brief Do any setup before running the game.
  void Initialize();

  //! \brief Run the game loop.
  void Run();

  //! \brief Do any cleanup.
  void Finalize();

  void SetFrame(CGRect window_frame) { window_frame_ = window_frame; }
  [[nodiscard]] CGRect GetFrame() const { return window_frame_; }

  //! \brief Update step.
  void Update(float delta);

  //! \brief Set up the texture.
  void InitializeTexture(TextureBitmap& texture_bitmap, MTL::Device* device) const;

  //! \brief Get the world that the game manages.
  [[nodiscard]] world::World& GetWorld() { return *world_; }

private:
  void setup();

  void setDelegates();

  //! \brief Draw the visible world to a texture.
  void draw(TextureBitmap& texture_bitmap) const;

  void drawTexture() const;

  static void simulation(Game* game);

  const uint32_t texture_width_;
  const uint32_t texture_height_;

  std::mutex world_mutex_{};

  //! \brief Pointer to the world.
  std::unique_ptr<world::World> world_{};

  CGRect window_frame_{};

  std::unique_ptr<GameAppDelegate> application_{};

  std::thread simulation_thread_;

  volatile bool is_initialized_ = false;
  bool is_running_ = false;
};

}  // namespace pixelengine::app