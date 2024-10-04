//
// Created by Nathaniel Rupprecht on 9/9/24.
//

#include "pixelengine/application/Game.h"
// Other files.
#include <pixelengine/graphics/ShaderProgram.h>

#include "pixelengine/input/Input.h"

namespace pixelengine::app {


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

void Game::update(float delta) {
  using namespace pixelengine::world;

  // Update the input object.
  input::Input::Update();

  for (auto& node : nodes_) {
    node->removeQueuedChildren();
  }

  for (auto& node : nodes_) {
    node->addQueuedChildren();
  }

  updateWorldPhysics(delta);

  // Update the physics of entities in the world.
  for (const auto& bodies : nodes_) {
    bodies->updatePhysics(delta);
  }

  updateWorld(delta);

  // Call the update logic for all entities in the world.
  for (const auto& entity : nodes_) {
    entity->update(delta);
  }

  // Set Input object to be ready for the next update.
  input::Input::Checkpoint();
}

std::optional<std::array<long long, 2>> Game::getCursorPosition() const {
  // Y measured from top left.
  auto cursor          = input::Input::GetCursorPosition();
  auto [width, height] = GetScreenResolution();
  cursor.y             = height - cursor.y;

  auto frame = application_->GetFrame();

  auto origin = frame.origin;
  auto size   = frame.size;

  // TODO: The window's upper bar is 28 pixels tall.
  size.height -= 28;

  if (origin.x <= cursor.x                 //
      && cursor.x < origin.x + size.width  //
      && origin.y <= cursor.y              //
      && cursor.y < origin.y + size.height)
  {
    // Normalize cursor position.
    auto x = static_cast<long long>((cursor.x - origin.x) / size.width * texture_width_);
    auto y = static_cast<long long>((cursor.y - origin.y) / size.height * texture_height_);

    return std::array {x, y};
  }
  return {};
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

  world_texture_.Initialize(texture_width_, texture_height_, device);
  main_drawable_ = std::make_shared<graphics::RectangularDrawable>(
      shader_programs_.back().get(),
      std::make_unique<graphics::TextureWrapper>(world_texture_.GetTexture()),
      device);

  // Add the main drawable to the renderer.
  // application_->GetViewDelegate().GetRenderer().AddDrawable(main_drawable_);

  // Set the callback
  if (!run_simulation_independently_) {
    // If the simulation is tied to the main thread, then update the world and then draw the texture.
    application_->GetViewDelegate().SetDrawViewCallback([this](float delta) {
      update(delta);
      drawTexture();
    });
  }

  application_->GetViewDelegate().SetRenderCallback(
      [this](MTL::RenderCommandEncoder* render_command_encoder) {
        drawTexture();

        for (auto& node : nodes_) {
          node->_draw(render_command_encoder);
        }

        // AsNode gets around the fact that overridden Drawable::_draw is private.
        main_drawable_->AsNode()._draw(render_command_encoder);
      });
}

void Game::updateWorldPhysics(float delta) {
  world_->Update(delta);
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
  // Update the metal texture behind the texture bitmap.
  texture.Update();
}

void Game::drawTexture() const {
  draw(world_texture_);
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