//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#include <utility>

#include "pixelengine/application/AppDelegate.h"

#include <Lightning/Lightning.h>
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

bool GameAppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* pSender) {
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

  renderer_->Draw(view);
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


void Renderer::Draw(MTK::View* view) {
  NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

  auto cmd_buffer                                   = command_queue_->commandBuffer();
  MTL::RenderPassDescriptor* descriptor             = view->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* render_command_encoder = cmd_buffer->renderCommandEncoder(descriptor);

  TextureBitmap texture_bitmap(1024, 1024, device_);
  auto&& color_attachment = descriptor->colorAttachments()->object(0);
  descriptor->colorAttachments()->object(0)->setTexture(texture_bitmap.GetTexture());
  color_attachment->setLoadAction(MTL::LoadAction::LoadActionClear);
  color_attachment->setStoreAction(MTL::StoreAction::StoreActionStore);

  for (auto& drawable : drawables_) {
    drawable->Draw(render_command_encoder);
  }

  // Render pass is complete.
  render_command_encoder->endEncoding();

  // Finish up
  cmd_buffer->presentDrawable(view->currentDrawable());
  cmd_buffer->commit();

  pPool->release();
}

}  // namespace pixelengine::app