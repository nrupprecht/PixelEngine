//
// Created by Nathaniel Rupprecht on 9/9/24.
//

#include "pixelengine/application/Game.h"
// Other files.
#include <pixelengine/graphics/ShaderProgram.h>

#include "pixelengine/input/Input.h"

namespace pixelengine::app {

// TODO: Move these.
world::FallingPhysics falling {};
world::LiquidPhysics liquid {};
world::Stationary stationary {};

constexpr Color SAND_COLORS[] = {
    Color(204, 171, 114),
    Color(200, 168, 113),
    Color(179, 149, 100),
    Color(179, 149, 100),
};

// A background color.
constexpr Color BACKGROUND = Color(240, 228, 228);

// ===========================================================================================================


void Game::Initialize() {
  LL_REQUIRE(0 < window_dimensions_.Area(), "dimensions must be positive");
  input::Input::Initialize();

  application_ = std::make_unique<GameAppDelegate>(window_dimensions_);
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

void Game::Finalize() {}


void Game::Update(float delta) {
  using namespace pixelengine::world;

  // Update the input object.
  input::Input::Update();

  static unsigned brush_type = 0;
  if (input::Input::IsJustPressed(0x2) /* D */) {
    brush_type += 1;
    brush_type = brush_type % 3;
  }

  // Update the world based on input.
  {
    // std::lock_guard lock(world_mutex_);

    if (input::Input::IsLeftMousePressed()) {
      // Y measured from top left.
      auto cursor          = input::Input::GetCursorPosition();
      auto [width, height] = GetScreenResolution();
      cursor.y             = height - cursor.y;

      auto frame = application_->GetFrame();

      auto origin = frame.origin;
      auto size   = frame.size;

      // TODO: The window's upper bar is 28 pixels tall.
      size.height -= 28;
      if (origin.x <= cursor.x && cursor.x < origin.x + size.width && origin.y <= cursor.y
          && cursor.y < origin.y + size.height)
      {
        // Normalize cursor position.
        auto x = static_cast<long long>((cursor.x - origin.x) / size.width * texture_width_);
        auto y = static_cast<long long>((cursor.y - origin.y) / size.height * texture_height_);

        // Generate randomly in a circle
        int radius = 5;
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

                Square square;

                if (brush_type == 0) {
                  square            = Square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
                  square.velocity_y = -50;
                }
                else if (brush_type == 1) {
                  square            = Square(true, Color::FromFloats(0., 0., 1.), &WATER, &liquid);
                  square.velocity_y = -50;
                }
                else if (brush_type == 2) {
                  square =
                      Square(true, Color(randi(30, 60), randi(30, 60), randi(30, 60)), &DIRT, &stationary);
                }
                world_->SetSquare(x + i, y + j, square);
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

void Game::setup() {
  using namespace pixelengine::world;

  for (auto j = 0u; j < world_->GetHeight(); ++j) {
    for (auto i = 0u; i < world_->GetWidth(); ++i) {
      auto r = randf();
      if (r < 0.2) {
        // if (i < world_->GetWidth() / 2) {
        auto c = randf();
        Square sand_square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
        world_->SetSquare(i, j, sand_square);
      }
      else {
        world_->SetSquare(i, j, Square(false, BACKGROUND, &AIR, nullptr));
      }
    }
  }
}

void Game::setDelegates() {
  std::string shader = R"(
      #include <metal_stdlib>
      using namespace metal;

      struct VertexData
       {
           float3 position;
           float2 texcoord;
       };

       struct VertexFragment
       {
         float4 position [[position]];
         float2 texcoord;
       };

      VertexFragment vertex vertexMain( device const VertexData* vertexData [[buffer(0)]], uint vertexID [[vertex_id]] )
      {
        VertexFragment o;
        o.position = float4( vertexData[ vertexID ].position, 1.0 );
        o.texcoord = vertexData[ vertexID ].texcoord;
        return o;
      }

      half4 fragment fragmentMain(
        VertexFragment in [[stage_in]],
        texture2d<half, access::sample> tex [[texture(0)]]
      )
      {
        constexpr float scale = 0.01;

        constexpr sampler s( address::repeat, filter::nearest );
        half3 texel = tex.sample( s, in.texcoord ).rgb;
        return half4( texel, 1.0 );
      }
  )";

  auto device  = application_->GetDevice();
  auto program = std::make_unique<graphics::ShaderProgram>(device, shader, "vertexMain", "fragmentMain");
  shader_programs_.push_back(std::move(program));
  main_drawable_ = std::make_shared<graphics::RectangularDrawable>(
      shader_programs_.back().get(), texture_width_, texture_height_, device);


  // Add the main drawable to the renderer.
  application_->GetViewDelegate().GetRenderer().AddDrawable(main_drawable_);

  // Set the callback
  if (run_simulation_independently_) {
    application_->GetViewDelegate().SetDrawViewCallback(
        [this]([[maybe_unused]] float delta) { drawTexture(); });
  }
  else {
    // If the simulation is tied to the main thread, then update the world and then draw the texture.
    application_->GetViewDelegate().SetDrawViewCallback([this](float delta) {
      Update(delta);
      drawTexture();
    });
  }
}

void Game::draw(TextureBitmap& texture) const {
  // Update pixels to render the world.
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
  draw(main_drawable_->GetTextureBitmap());
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

    game->Update(delta);
    last_time = t0;

    // TODO: Intentional sleep.
    // Put this_thread to sleep for 100 microseconds.
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
}


}  // namespace pixelengine::app