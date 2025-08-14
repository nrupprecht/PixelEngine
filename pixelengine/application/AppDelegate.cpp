//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#include <utility>

#include "pixelengine/application/AppDelegate.h"

#include <Lightning/Lightning.h>

#include "pixelengine/graphics/RectangularDrawable.h"
// Other files.

namespace pixelengine::app {

// ===========================================================================================================
//  GameAppDelegate.
// ===========================================================================================================

GameAppDelegate::~GameAppDelegate() {
  view_->release();
  window_->release();
  device_->release();
}

NS::Menu* GameAppDelegate::createMenuBar() {
  using NS::StringEncoding::UTF8StringEncoding;

  NS::Menu* main_menu         = NS::Menu::alloc()->init();
  NS::MenuItem* app_menu_item = NS::MenuItem::alloc()->init();
  NS::Menu* app_menu          = NS::Menu::alloc()->init(NS::String::string("Appname", UTF8StringEncoding));

  NS::String* app_name = NS::RunningApplication::currentApplication()->localizedName();
  NS::String* quit_item_name =
      NS::String::string("Quit ", UTF8StringEncoding)->stringByAppendingString(app_name);

  SEL quit_callback =
      NS::MenuItem::registerActionCallback("appQuit", [](void*, SEL, const NS::Object* pSender) {
        auto pApp = NS::Application::sharedApplication();
        pApp->terminate(pSender);
      });

  NS::MenuItem* app_quit_item =
      app_menu->addItem(quit_item_name, quit_callback, NS::String::string("q", UTF8StringEncoding));
  app_quit_item->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
  app_menu_item->setSubmenu(app_menu);

  NS::MenuItem* window_menu_item = NS::MenuItem::alloc()->init();
  NS::Menu* window_menu          = NS::Menu::alloc()->init(NS::String::string("Window", UTF8StringEncoding));

  SEL close_window_callback =
      NS::MenuItem::registerActionCallback("windowClose", [](void*, SEL, const NS::Object*) {
        auto pApp = NS::Application::sharedApplication();
        pApp->windows()->object<NS::Window>(0)->close();
      });
  NS::MenuItem* close_window_item =
      window_menu->addItem(NS::String::string("Close Window", UTF8StringEncoding),
                           close_window_callback,
                           NS::String::string("w", UTF8StringEncoding));
  close_window_item->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);


  window_menu_item->setSubmenu(window_menu);

  main_menu->addItem(app_menu_item);
  main_menu->addItem(window_menu_item);

  app_menu_item->release();
  window_menu_item->release();
  app_menu->release();
  window_menu->release();

  return main_menu->autorelease();
}

void GameAppDelegate::applicationWillFinishLaunching(NS::Notification* pNotification) {
  NS::Menu* pMenu = createMenuBar();
  auto app        = reinterpret_cast<NS::Application*>(pNotification->object());
  app->setMainMenu(pMenu);
  app->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void GameAppDelegate::applicationDidFinishLaunching(NS::Notification* pNotification) {
  CGRect frame {.origin = {100.0, 100.0},
                .size   = {static_cast<CGFloat>(window_dimensions_.width),
                           static_cast<CGFloat>(window_dimensions_.height)}};

  window_ = NS::Window::alloc()->init(
      frame,
      NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled | NS::WindowStyleMaskMiniaturizable,
      NS::BackingStoreBuffered,
      false);

  device_ = MTL::CreateSystemDefaultDevice();

  view_ = MTK::View::alloc()->init(frame, device_);
  view_->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
  view_->setClearColor(MTL::ClearColor::Make(0.1, 0.1, 0.1, 1.0));
  view_->setDepthStencilPixelFormat(MTL::PixelFormat::PixelFormatDepth16Unorm);
  view_->setClearDepth(1.0f);

  view_delegate_ = std::make_unique<GameViewDelegate>(device_);
  view_->setDelegate(view_delegate_.get());

  window_->setContentView(view_);
  window_->setTitle(NS::String::string(window_title_.c_str(), NS::StringEncoding::UTF8StringEncoding));

  window_->makeKeyAndOrderFront(nullptr);

  auto app = reinterpret_cast<NS::Application*>(pNotification->object());
  app->activateIgnoringOtherApps(true);

  // Run the callback, now that the window is set up.
  if (did_finish_launching_callback_) {
    did_finish_launching_callback_();
  }
}

bool GameAppDelegate::applicationShouldTerminateAfterLastWindowClosed([[maybe_unused]] NS::Application* pSender) {
  return true;
}

// ===========================================================================================================
//  GameViewDelegate.
// ===========================================================================================================

GameViewDelegate::GameViewDelegate(MTL::Device* devide)
    : MTK::ViewDelegate()
    , renderer_(std::make_unique<Renderer>(devide)) {}

void GameViewDelegate::drawInMTKView(MTK::View* view) {
  frame_timer_.Mark();

  if (draw_view_callback_) {
    auto delta = static_cast<float>(frame_timer_.GetLastElapsedSeconds());
    // Callback to the game to do anything that should be done before drawing.
    draw_view_callback_(delta);
  }

  // renderer_->Draw(view);

  if (render_callback_) {
    renderer_->BeginDraw(view);
    // Render main objects in the first pass.
    auto render_command_encoder = renderer_->BeginCommand();
    render_callback_(render_command_encoder);
    renderer_->EndCommand(render_command_encoder);
    // Here, other stages could be rendered.
    // End, present drawable.
    renderer_->EndDraw();
  }
}

// ===========================================================================================================
//  Renderer.
// ===========================================================================================================

Renderer::Renderer(MTL::Device* device) : device_(device->retain()) {
  command_queue_ = device_->newCommandQueue();
}

Renderer::~Renderer() {
  command_queue_->release();
  device_->release();
}

void Renderer::BeginDraw(MTK::View* view) {
  current_view_       = view;
  current_pool_       = NS::AutoreleasePool::alloc()->init();
  current_cmd_buffer_ = command_queue_->commandBuffer();
  current_descriptor_ = view->currentRenderPassDescriptor();

  auto&& color_attachment = current_descriptor_->colorAttachments()->object(0);
  // NOTE: If you want to render to a texture other than the current drawable's texture, set that here.
  // color_attachment->setTexture(texture_bitmap.GetTexture());

  color_attachment->setLoadAction(MTL::LoadAction::LoadActionClear);
  color_attachment->setStoreAction(MTL::StoreAction::StoreActionStore);
  color_attachment->setClearColor(MTL::ClearColor(0., 0., 0., 1.));
}

MTL::RenderCommandEncoder* Renderer::BeginCommand() const {
  return current_cmd_buffer_->renderCommandEncoder(current_descriptor_);
}

void Renderer::EndCommand(MTL::RenderCommandEncoder* render_command_encoder) const {
  // Render pass is complete.
  render_command_encoder->endEncoding();
}

void Renderer::EndDraw() {
  auto current_drawable = current_view_->currentDrawable();
  current_cmd_buffer_->presentDrawable(current_drawable);
  current_cmd_buffer_->commit();

  if (current_pool_) {
    current_pool_->release();
  }

  // Reset the state.
  current_descriptor_ = {};
  current_cmd_buffer_ = {};
  current_pool_       = {};
  current_view_       = {};
}

// void Renderer::Draw(MTK::View* view) {
//   NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();
//
//   // TextureBitmap texture_bitmap(1024, 1024, device_);
//
//   auto cmd_buffer                       = command_queue_->commandBuffer();
//   MTL::RenderPassDescriptor* descriptor = view->currentRenderPassDescriptor();
//
//   auto&& color_attachment = descriptor->colorAttachments()->object(0);
//   // NOTE: If you want to render to a texture other than the current drawable's texture, set that here.
//   // color_attachment->setTexture(texture_bitmap.GetTexture());
//
//   color_attachment->setLoadAction(MTL::LoadAction::LoadActionClear);
//   color_attachment->setStoreAction(MTL::StoreAction::StoreActionStore);
//   color_attachment->setClearColor(MTL::ClearColor(0., 0., 0., 1.));
//
//   MTL::RenderCommandEncoder* render_command_encoder = cmd_buffer->renderCommandEncoder(descriptor);
//   for (auto& drawable : drawables_) {
//     drawable->Draw(render_command_encoder);
//   }
//
//   // render_command_encoder->endEncoding();
//
//
//   // Second render pass - need a new encoder.
//   // This is necessary, so we write to the drawable instead of the texture.
//   // color_attachment->setTexture(view->currentDrawable()->texture());
//   // MTL::RenderCommandEncoder* second_render_command_encoder = cmd_buffer->renderCommandEncoder(descriptor);
//
//   // Second pass.
//   static std::string body = R"(
//     using namespace metal;
//
//     struct VertexData {
//          float3 position;
//          float2 texcoord;
//     };
//
//     struct VertexFragment {
//        float4 position [[position]];
//        float2 texcoord;
//     };
//
//     VertexFragment vertex vertexMain(device const VertexData* vertexData [[buffer(0)]], uint vertexID [[vertex_id]] )
//     {
//       VertexFragment o;
//       o.position = float4( vertexData[ vertexID ].position, 1.0 );
//       o.texcoord = vertexData[ vertexID ].texcoord;
//       return o;
//     }
//
//     fragment half4 fragment_shader(VertexFragment in [[stage_in]],
//                                    texture2d<half, access::sample> tex [[texture(0)]]) {
//
//       constexpr sampler s( address::repeat, filter::nearest );
//
//       float2 pos(in.position[0], in.position[1]);
//       float2 uv = in.texcoord;
//
//       // float2 uv(in.position[0], in.position[1]);
//       // Distortion logic here, e.g., manipulate uv
//
//       // return half4(cos(500 * 2 * 3.14159 * in.position[0]), 1.0, 1.0, 1.0);
//
//       float multiplier = 0.5;
//       if (0.4 < uv.x && uv.x < 0.6 && 0.4 < uv.y && uv.y < 0.6) {
//         uv.x += sin(uv.y * 50.0) * 0.005;
//         multiplier *= 2.0;
//       }
//       return tex.sample(s, uv) * multiplier;
//     })";
//
//   // static graphics::ShaderProgram program(device_, body, "vertexMain", "fragment_shader");
//   // auto texture_container = std::make_unique<graphics::TextureWrapper>(texture_bitmap.GetTexture());
//   // graphics::RectangularDrawable drawable(&program, std::move(texture_container), device_);
//   // drawable.SetWidth(0.2);
//   // drawable.SetHeight(0.2);
//   // drawable.Draw(render_command_encoder); //
//
//   // Render pass is complete.
//   render_command_encoder->endEncoding();  //
//
//   // Finish up
//   auto current_drawable = view->currentDrawable();
//   cmd_buffer->presentDrawable(current_drawable);
//   cmd_buffer->commit();
//
//   pPool->release();
// }

}  // namespace pixelengine::app