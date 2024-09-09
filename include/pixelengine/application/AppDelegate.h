//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <utility>

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

#include "pixelengine/TextureBitmap.h"

namespace pixelengine::app {

struct IndexedPrimitive {
  MTL::Buffer* vertex_data;
  MTL::Buffer* index_data;
};

//! \brief Object that encapsulates the rendering of the game.
class Renderer {
public:
  explicit Renderer(MTL::Device* pDevice);
  ~Renderer();
  void buildShaders();
  void buildDepthStencilStates();
  void buildTextures();
  void buildBuffers();
  void draw(MTK::View* pView);

  [[nodiscard]] TextureBitmap& GetTexture() { return texture_bitmap_; }

private:

  //! \brief The texture bitmap that is rendered to the screen.
  TextureBitmap texture_bitmap_;

  MTL::Device* device_;
  MTL::CommandQueue* command_queue_;
  MTL::Library* shader_library_;
  MTL::RenderPipelineState* pipeline_state_;
  MTL::DepthStencilState* depth_stencil_state_;

  MTL::Buffer* _pVertexDataBuffer;
  MTL::Buffer* _pIndexBuffer;
};

class GameViewDelegate : public MTK::ViewDelegate {
public:
  explicit GameViewDelegate(MTL::Device* pDevice);

  //! \brief Virtual function to draw the view for each frame.
  void drawInMTKView(MTK::View* pView) override;

  void SetGameCallback(std::function<void(float)> callback) {
    game_callback_ = std::move(callback);
  }

  Renderer& GetRenderer() {
    return *renderer_;
  }

private:
  //! \brief Pointer to the rendering logic.
  std::unique_ptr<Renderer> renderer_;

  //! \brief Callback to hook the game into the rendering loop.
  std::function<void(float)> game_callback_;

  //! \brief Timing information.
  std::chrono::high_resolution_clock::time_point last_render_time_ {};
  double last_frame_time_us_ {};
  std::size_t num_frames_ {};
};

class GameAppDelegate : public NS::ApplicationDelegate {
public:
  GameAppDelegate() = default;
  ~GameAppDelegate();

  NS::Menu* createMenuBar();

  void applicationWillFinishLaunching(NS::Notification* pNotification) override;
  void applicationDidFinishLaunching(NS::Notification* pNotification) override;
  bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* pSender) override;

  CGRect GetFrame() {
    return window_->frame();
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
  NS::Window* window_;
  MTK::View* view_;
  MTL::Device* device_;
  std::unique_ptr<GameViewDelegate> view_delegate_;

  std::function<void()> did_finish_launching_callback_;

  std::string window_title_ = "Cool game";
};

}  // namespace pixelengine::app