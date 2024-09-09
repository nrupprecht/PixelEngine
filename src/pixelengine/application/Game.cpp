//
// Created by Nathaniel Rupprecht on 9/9/24.
//

#include "pixelengine/application/Game.h"
// Other files.
#include "pixelengine/input/Input.h"

namespace pixelengine::app {

// TODO: Move these.
world::FallingPhysics falling {};
world::LiquidPhysics liquid {};

constexpr Color SAND_COLORS[] = {
    Color(204, 171, 114),
    Color(200, 168, 113),
    Color(179, 149, 100),
    Color(179, 149, 100),
};

// A background color.
constexpr Color BACKGROUND = Color(240, 228, 228);


void Game::Initialize() {
  input::Input::Initialize();

  application_ = std::make_unique<GameAppDelegate>();
  application_->SetDidFinishLaunchingCallback([this]() {
    setDelegates();
    // Notify the game object that it is ready to run.
    is_initialized_ = true;
  });
}

void Game::Run() {
  NS::AutoreleasePool* pAutoreleasePool = NS::AutoreleasePool::alloc()->init();

  NS::Application* application = NS::Application::sharedApplication();
  application->setDelegate(application_.get());

  // Set up the world.
  setup();

  // Run the application loop.
  is_running_        = true;
  simulation_thread_ = std::thread(simulation, this);
  application->run();

  pAutoreleasePool->release();
}

void Game::Finalize() {}


void Game::Update(float delta) {
  using namespace pixelengine::world;

  // Update the input object.
  input::Input::Update();

  // Update the world based on input.
  {
    // std::lock_guard lock(world_mutex_);

    if (input::Input::IsLeftMousePressed()) {
      auto cursor = input::Input::GetCursorPosition();

      auto frame  = application_->GetFrame();
      auto origin = frame.origin;
      auto size   = frame.size;

      if (cursor.x < size.width && cursor.y < size.height) {
        // Normalize cursor position.
        auto x = static_cast<long long>((cursor.x - origin.x) / size.width * texture_width_);
        auto y = static_cast<long long>((1. - (cursor.y - origin.y) / size.height) * texture_height_);

        // Generate randomly in a circle
        int radius = 10;
        for (int i = -radius; i < radius; ++i) {
          for (int j = -radius; j < radius; ++j) {
            if (i * i + j * j < radius * radius && 0 <= x + i && x + i < world_->GetWidth() && 0 <= y + j
                && y + j < world_->GetHeight())
            {
              if (world_->GetSquare(x + i, y + j).is_occupied) {
                continue;
              }
              if (randf() < 0.7) {
                auto c = randf();
                Square sand_square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
                sand_square.velocity_y = -50;
                world_->SetSquare(x + i, y + j, sand_square);
              }
            }
          }
        }

      }
    }

    world_->Update(delta);
  }

  input::Input::Checkpoint();
}

void Game::InitializeTexture(TextureBitmap& texture_bitmap, MTL::Device* device) const {
  texture_bitmap.Initialize(texture_width_, texture_height_, device);
}


void Game::setup() {
  using namespace pixelengine::world;

  for (auto j = 0u; j < world_->GetHeight(); ++j) {
    for (auto i = 0u; i < world_->GetWidth(); ++i) {
      auto r = randf();
      if (r < 0.2) {
        // if (i < world_->GetWidth() / 2) {
        //   auto c = randf();
        //   Square sand_square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
        //   world_->SetSquare(i, j, sand_square);
        // }
        // else {
          Square water_square(true, Color::FromFloats(0., 0., 1.), &WATER, &liquid);
          world_->SetSquare(i, j, water_square);
        // }
      }
      else {
        world_->SetSquare(i, j, Square(false, BACKGROUND, &AIR, nullptr));
      }
    }
  }
}

void Game::setDelegates() {
  // Set up the texture.
  application_->GetViewDelegate().GetRenderer().GetTexture().Initialize(
      texture_width_, texture_height_, application_->GetDevice());

  // Set the callback
  application_->GetViewDelegate().SetGameCallback([this](float delta) { drawTexture(); });
}

void Game::draw(TextureBitmap& texture) const {
  // std::lock_guard world_lock(world_mutex_);

  // Update pixels to render the world_->
  for (auto j = 0u; j < texture.GetHeight(); ++j) {
    for (auto i = 0u; i < texture.GetWidth(); ++i) {
      auto x        = i;
      auto y        = texture.GetHeight() - 1 - j;
      auto&& square = world_->GetSquare(i, j);
      texture.SetPixel(x, y, square.color);
    }
  }
}

void Game::drawTexture() const {
  // TODO: Locking.
  auto& texture = application_->GetViewDelegate().GetRenderer().GetTexture();
  draw(texture);
}

void Game::simulation(Game* game) {
  while (!game->is_initialized_) {}

  std::chrono::high_resolution_clock::time_point last_time = std::chrono::high_resolution_clock::now();
  while (game->is_running_) {
    std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed             = t0 - last_time;
    auto delta                                        = static_cast<float>(elapsed.count());

    game->Update(delta);
    last_time = t0;

    // TODO: Intentional sleep.
    // Put this_thread to sleep for 100 microseconds.
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
}


}  // namespace pixelengine::app