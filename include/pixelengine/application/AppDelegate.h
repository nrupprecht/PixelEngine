//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include "pixelengine/graphics/Drawable.h"
#include "pixelengine/utility/FrameTimer.h"


namespace pixelengine::app {


//! \brief Object that encapsulates the rendering of the game.
class Renderer {
public:
  explicit Renderer(MTL::Device* pDevice);
  ~Renderer();

  void Draw(MTK::View* pView);

  void AddDrawable(std::shared_ptr<graphics::Drawable> drawable) {
    drawables_.push_back(std::move(drawable));
  }

private:
  MTL::Device* device_{};
  MTL::CommandQueue* command_queue_{};

  std::vector<std::shared_ptr<graphics::Drawable>> drawables_;
};

class GameViewDelegate : public MTK::ViewDelegate {
public:
  explicit GameViewDelegate(MTL::Device* device);

  //! \brief Virtual function to draw the view for each frame.
  void drawInMTKView(MTK::View* pView) override;

  void SetDrawViewCallback(std::function<void(float)> callback) {
    draw_view_callback_ = std::move(callback);
  }

  [[nodiscard]] Renderer& GetRenderer() const {
    return *renderer_;
  }

private:
  //! \brief Pointer to the rendering logic.
  std::unique_ptr<Renderer> renderer_;

  //! \brief Callback to hook the game into the rendering loop.
  std::function<void(float)> draw_view_callback_;

  //! \brief Timing information.
  utility::FrameTimer frame_timer_;
};

//! \brief The main delegate that the game runs to manage rendering and GPU resources.
class GameAppDelegate : public NS::ApplicationDelegate {
public:
  explicit GameAppDelegate(Dimensions window_dimensions)
    : window_dimensions_(window_dimensions) {}

  GameAppDelegate(std::size_t window_width, std::size_t window_height)
    : window_dimensions_({window_width, window_height}) {}

  ~GameAppDelegate() override;

  NS::Menu* createMenuBar();

  void applicationWillFinishLaunching(NS::Notification* pNotification) override;
  void applicationDidFinishLaunching(NS::Notification* pNotification) override;
  bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* pSender) override;

  CGRect GetFrame() {
    return window_->frame();
  }

  NS::Window* GetWindow() {
    return window_;
  }

  GameViewDelegate& GetViewDelegate() {
    return *view_delegate_;
  }

  MTL::Device* GetDevice() {
    return device_;
  }

  void SetDidFinishLaunchingCallback(std::function<void()> callback) {
    did_finish_launching_callback_ = std::move(callback);
  }

private:
  Dimensions window_dimensions_;

  NS::Window* window_{};
  MTK::View* view_{};
  MTL::Device* device_{};

  //! \brief The view delegate.
  std::unique_ptr<GameViewDelegate> view_delegate_{};

  std::function<void()> did_finish_launching_callback_{};

  std::string window_title_ = "Cool game";
};

}  // namespace pixelengine::app