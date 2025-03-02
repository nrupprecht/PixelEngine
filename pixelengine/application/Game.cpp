//
// Created by Nathaniel Rupprecht on 9/9/24.
//

#include "pixelengine/application/Game.h"
// Other files.
#include "pixelengine/graphics/ShaderStore.h"
#include "pixelengine/input/Input.h"

namespace pixelengine::app {

Game::Game(Dimensions window_dimensions) : window_dimensions_(window_dimensions) {}

Game::~Game() {
  // Clean up shader program store.
  graphics::ShaderStore::cleanGlobalInstance();
}

void Game::Initialize() {
  LL_REQUIRE(0 < window_dimensions_.Area(), "dimensions must be positive");
  input::Input::Initialize();

  application_ = std::make_unique<GameAppDelegate>(window_dimensions_);
  application_->SetDidFinishLaunchingCallback([this]() {
    setDelegates();

    // Set up the world.
    setup();

    // Call the initialization hook after the application has launched.
    initialize();

    // Notify the game object that it is ready to run.
    is_initialized_ = true;
  });
}

void Game::Finalize() {
  finalize();
}

void Game::Run() {
  NS::AutoreleasePool* pAutoreleasePool = NS::AutoreleasePool::alloc()->init();

  NS::Application* application = NS::Application::sharedApplication();
  application->setDelegate(application_.get());

  // Run the application loop.
  if (run_simulation_independently_) {
    is_running_ = true;
    // Run the world simulation in a separate thread. `is_running_` is used to synchronize.
    simulation_thread_ = std::thread(simulation, this);
    application->run();
  }
  else {
    is_running_ = true;
    application->run();
  }

  pAutoreleasePool->release();
}

void Game::update(float delta) {
  using namespace pixelengine::world;

  // Potentially limit how large delta can be.
  delta = std::min(1.f / 60.f, delta);

  // Update the input object.
  input::Input::Update(application_->GetFrame());

  for (auto& node : nodes_) {
    node->removeQueuedChildren();
  }

  for (auto& node : nodes_) {
    node->addQueuedChildren();
  }

  // Update the physics of entities in the world.
  for (const auto& bodies : nodes_) {
    bodies->updatePhysics(delta, nullptr /* No world */);
  }

  // Call the update logic for all entities in the world.
  for (const auto& entity : nodes_) {
    entity->update(delta);
  }

  // Set Input object to be ready for the next update.
  input::Input::Checkpoint();
}

void Game::addNode(std::unique_ptr<Node> node) {
  if (node) {
    auto& ptr = nodes_.emplace_back(std::move(node));
    LOG_SEV(Trace) << "Added node [" << ptr << "] to game.";
  }
  else {
    LOG_SEV(Debug) << "Warning: Trying to add null node to Game.";
  }
}

void Game::setDelegates() {
  // Set the callback
  if (!run_simulation_independently_) {
    // If the simulation is tied to the main thread, then update the world and then draw the texture.
    application_->GetViewDelegate().SetDrawViewCallback([this](float delta) { update(delta); });
  }

  application_->GetViewDelegate().SetRenderCallback(
      [this](MTL::RenderCommandEncoder* render_command_encoder) {
        for (auto& node : nodes_) {
          node->draw(render_command_encoder, {} /* no window context */, {} /* No parent offset. */);
        }
      });

  graphics::ShaderStore::makeGlobalInstance(application_->GetDevice());
}

void Game::simulation(Game* game) {
  // Spin.
  while (!game->is_initialized_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  std::chrono::high_resolution_clock::time_point last_time = std::chrono::high_resolution_clock::now();
  while (game->is_running_) {
    std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed             = t0 - last_time;
    auto delta                                        = static_cast<float>(elapsed.count());

    game->update(delta);
    last_time = t0;

    // TODO: Intentional sleep.
    // Put this_thread to sleep for 100 microseconds.
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
}


}  // namespace pixelengine::app