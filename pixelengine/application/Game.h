//
// Created by Nathaniel Rupprecht on 9/9/24.
//

#pragma once

#include <thread>
#include <list>

#include "pixelengine/application/AppDelegate.h"
#include "pixelengine/world/World.h"
#include "pixelengine/node/Scene.h"
#include "pixelengine/graphics/RectangularDrawable.h"

namespace pixelengine::app {

//! \brief Class for the game engine.
class Game {
public:
  Game(Dimensions window_dimensions);

  ~Game();

  //! \brief Do any setup before running the game.
  void Initialize();

  void Finalize();

  //! \brief Run the game loop.
  void Run();

  void SetFrame(CGRect window_frame) { window_frame_ = window_frame; }
  [[nodiscard]] CGRect GetFrame() const { return window_frame_; }

protected:
  //! \brief Load resources.
  virtual void initialize() {}

  //! \brief Set up the game world.
  virtual void setup() {}

  virtual void finalize() {}

  void addNode(std::unique_ptr<Node> node);

private:
  void setDelegates();

  //! \brief Update step. Calls all the other update functions.
  void update(float delta);

  static void simulation(Game* game);

  // ===========================================================================
  // Private data.
  // ===========================================================================

  //! \brief The dimensions of the window.
  Dimensions window_dimensions_;

  CGRect window_frame_{};

  //! \brief The application delegate that runs the GUI.
  std::unique_ptr<GameAppDelegate> application_{};

  std::deque<std::unique_ptr<graphics::ShaderProgram>> shader_programs_;

  std::thread simulation_thread_;

  bool run_simulation_independently_ = false;

  volatile bool is_initialized_ = false;
  bool is_running_ = false;

  //! \brief The game scene.
  std::unique_ptr<Scene> scene_;
};

}  // namespace pixelengine::app