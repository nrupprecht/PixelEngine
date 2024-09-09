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
// 240 228 228

constexpr Color BACKGROUND = Color(240, 228, 228);


void Game::Initialize() {
  input::Input::Initialize();

  application_ = std::make_unique<GameAppDelegate>();
  application_->SetDidFinishLaunchingCallback([this]() { setDelegates(); });
}

void Game::Run() {
  NS::AutoreleasePool* pAutoreleasePool = NS::AutoreleasePool::alloc()->init();

  NS::Application* application = NS::Application::sharedApplication();
  application->setDelegate(application_.get());

  // Set up the world.
  setup();

  // Run the application loop.
  is_running_ = true;
  // simulation_thread_ = std::thread(simulation, this);
  application->run();

  pAutoreleasePool->release();
}

void Game::Finalize() {}


void Game::Update(float delta) {
  using namespace pixelengine::world;

  // Update the input object.
  input::Input::Update();

  // Update the world based on input.

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
          if (i * i + j * j < radius * radius && x + i >= 0 && x + i < world_->GetWidth() && y + j >= 0
              && y + j < world_->GetHeight())
          {
            auto r = randf();
            if (r < 0.7) {
              auto c = randf();
              Square sand_square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
              world_->SetSquare(x + i, y + j, sand_square);
            }
            else {
              world_->SetSquare(x + i, y + j, Square(false, BACKGROUND, &AIR, nullptr));
            }
          }
        }
      }

      world_->SetSquare(x, y, Square(true, SAND_COLORS[static_cast<int>(4 * randf())], &SAND, &falling));
    }
  }

  world_->Update(delta);

  // Draw the visible world to a texture.
  Draw(application_->GetViewDelegate().GetRenderer().GetTexture());
}

void Game::Draw(TextureBitmap& texture) {
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

void Game::InitializeTexture(TextureBitmap& texture_bitmap, MTL::Device* device) const {
  texture_bitmap.Initialize(texture_width_, texture_height_, device);
}


void Game::setup() {
  using namespace pixelengine::world;

  for (auto j = 0u; j < world_->GetHeight(); ++j) {
    for (auto i = 0u; i < world_->GetWidth(); ++i) {
      auto r = randf();
      if (r < 0.2) {
        if (i < world_->GetWidth() / 2) {
          auto c = randf();
          Square sand_square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
          world_->SetSquare(i, j, sand_square);
        }
        else {
          Square water_square(true, Color::FromFloats(0., 0., 1.), &WATER, &liquid);
          world_->SetSquare(i, j, water_square);
        }
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
  application_->GetViewDelegate().SetGameCallback([this](float delta) { Update(delta); });
}

void Game::simulation(Game* game) {
  std::chrono::high_resolution_clock::time_point last_time = std::chrono::high_resolution_clock::now();
  while (game->is_running_) {

    std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = t0 - last_time;
    auto delta = static_cast<float>(elapsed.count());

    game->Update(delta);

    last_time = t0;
  }

}


}  // namespace pixelengine::app