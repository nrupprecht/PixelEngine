//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#pragma once

#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

#include "pixelengine/TextureBitmap.h"
#include "pixelengine/world/World.h"

namespace pixelengine::app {


static constexpr size_t kMaxFramesInFlight = 3;

class Renderer {
public:
  Renderer(MTL::Device* pDevice, world::World* world);
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

  MTL::Device* _pDevice;
  MTL::CommandQueue* _pCommandQueue;
  MTL::Library* _pShaderLibrary;
  MTL::RenderPipelineState* _pPSO;
  MTL::DepthStencilState* _pDepthStencilState;

  MTL::Buffer* _pVertexDataBuffer;
  MTL::Buffer* _pInstanceDataBuffer[kMaxFramesInFlight];
  MTL::Buffer* _pCameraDataBuffer[kMaxFramesInFlight];
  MTL::Buffer* _pIndexBuffer;

  // TODO: Presumably, this is not really where this belongs, but I am putting it here for now.
  world::World* world_{};

  float _angle;
  int _frame;
  dispatch_semaphore_t _semaphore;
  static const int kMaxFramesInFlight;

  std::chrono::high_resolution_clock::time_point last_render_time_ {};
  double last_frame_time_us_ {};
  std::size_t num_frames_ {};
};

class MyMTKViewDelegate : public MTK::ViewDelegate {
public:
  MyMTKViewDelegate(MTL::Device* pDevice, world::World* world);
  virtual ~MyMTKViewDelegate() override;
  virtual void drawInMTKView(MTK::View* pView) override;

private:
  Renderer* _pRenderer;

  //! \brief Pointer to the world.
  world::World* world_;
};

class MyAppDelegate : public NS::ApplicationDelegate {
public:
  MyAppDelegate(std::size_t width, std::size_t height) : world_(width, height) {}
  ~MyAppDelegate();

  NS::Menu* createMenuBar();

  void applicationWillFinishLaunching(NS::Notification* pNotification) override;
  void applicationDidFinishLaunching(NS::Notification* pNotification) override;
  bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* pSender) override;

  world::World& GetWorld() { return world_; }

private:
  NS::Window* _pWindow;
  MTK::View* _pMtkView;
  MTL::Device* _pDevice;
  std::unique_ptr<MyMTKViewDelegate> _pViewDelegate;

  std::string window_title_ = "Cool game";

  //! \brief The game world.
  world::World world_;
};

}  // namespace pixelengine::app