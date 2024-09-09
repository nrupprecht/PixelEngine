//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#include "pixelengine/application/AppDelegate.h"
// Other files.
#include <pixelengine/input/Input.h>

// TODO: Move to another place.
namespace shader_types {

struct VertexData {
  simd::float3 position;
  simd::float2 texcoord;
};

}  // namespace shader_types

namespace pixelengine::app {


GameAppDelegate::~GameAppDelegate() {
  view_->release();
  window_->release();
  device_->release();
}

NS::Menu* GameAppDelegate::createMenuBar() {
  using NS::StringEncoding::UTF8StringEncoding;

  NS::Menu* pMainMenu        = NS::Menu::alloc()->init();
  NS::MenuItem* pAppMenuItem = NS::MenuItem::alloc()->init();
  NS::Menu* pAppMenu         = NS::Menu::alloc()->init(NS::String::string("Appname", UTF8StringEncoding));

  NS::String* appName = NS::RunningApplication::currentApplication()->localizedName();
  NS::String* quitItemName =
      NS::String::string("Quit ", UTF8StringEncoding)->stringByAppendingString(appName);
  SEL quitCb = NS::MenuItem::registerActionCallback("appQuit", [](void*, SEL, const NS::Object* pSender) {
    auto pApp = NS::Application::sharedApplication();
    pApp->terminate(pSender);
  });

  NS::MenuItem* pAppQuitItem =
      pAppMenu->addItem(quitItemName, quitCb, NS::String::string("q", UTF8StringEncoding));
  pAppQuitItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
  pAppMenuItem->setSubmenu(pAppMenu);

  NS::MenuItem* pWindowMenuItem = NS::MenuItem::alloc()->init();
  NS::Menu* pWindowMenu         = NS::Menu::alloc()->init(NS::String::string("Window", UTF8StringEncoding));

  SEL closeWindowCb = NS::MenuItem::registerActionCallback("windowClose", [](void*, SEL, const NS::Object*) {
    auto pApp = NS::Application::sharedApplication();
    pApp->windows()->object<NS::Window>(0)->close();
  });
  NS::MenuItem* pCloseWindowItem =
      pWindowMenu->addItem(NS::String::string("Close Window", UTF8StringEncoding),
                           closeWindowCb,
                           NS::String::string("w", UTF8StringEncoding));
  pCloseWindowItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);


  pWindowMenuItem->setSubmenu(pWindowMenu);

  pMainMenu->addItem(pAppMenuItem);
  pMainMenu->addItem(pWindowMenuItem);

  pAppMenuItem->release();
  pWindowMenuItem->release();
  pAppMenu->release();
  pWindowMenu->release();

  return pMainMenu->autorelease();
}

void GameAppDelegate::applicationWillFinishLaunching(NS::Notification* pNotification) {
  NS::Menu* pMenu       = createMenuBar();
  NS::Application* pApp = reinterpret_cast<NS::Application*>(pNotification->object());
  pApp->setMainMenu(pMenu);
  pApp->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void GameAppDelegate::applicationDidFinishLaunching(NS::Notification* pNotification) {
  CGRect frame {{100.0, 100.0}, {1024.0, 1024.0}};

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

  auto pApp = reinterpret_cast<NS::Application*>(pNotification->object());
  pApp->activateIgnoringOtherApps(true);

  if (did_finish_launching_callback_) {
    did_finish_launching_callback_();
  }
}

bool GameAppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* pSender) {
  return true;
}

GameViewDelegate::GameViewDelegate(MTL::Device* pDevice)
    : MTK::ViewDelegate()
    , renderer_(std::make_unique<Renderer>(pDevice)) {}


void GameViewDelegate::drawInMTKView(MTK::View* pView) {
  std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

  // Difference between last_time_ and now.
  std::chrono::duration<double> elapsed = t0 - last_render_time_;
  // Convert to microseconds.
  last_frame_time_us_ = elapsed.count() * 1'000'000.0;
  last_render_time_   = t0;
  ++num_frames_;

  if (game_callback_) {
    float delta = elapsed.count();
    game_callback_(delta);
  }

  renderer_->draw(pView);
}


Renderer::Renderer(MTL::Device* pDevice)
    : device_(pDevice->retain()){
  command_queue_ = device_->newCommandQueue();
  buildShaders();
  buildDepthStencilStates();
  buildTextures();
  buildBuffers();
}

Renderer::~Renderer() {
  shader_library_->release();
  depth_stencil_state_->release();
  _pVertexDataBuffer->release();
  _pIndexBuffer->release();
  pipeline_state_->release();
  command_queue_->release();
  device_->release();
}

void Renderer::buildShaders() {
  using NS::StringEncoding::UTF8StringEncoding;

  const char* shaderSrc = R"(
        #include <metal_stdlib>
        using namespace metal;

        struct VertexData
         {
             float3 position;
             float2 texcoord;
         };

         struct v2f
         {
           float4 position [[position]];
           float2 texcoord;
         };

        v2f vertex vertexMain( device const VertexData* vertexData [[buffer(0)]], uint vertexID [[vertex_id]] )
        {
          v2f o;
          o.position = float4( vertexData[ vertexID ].position, 1.0 );
          o.texcoord = vertexData[ vertexID ].texcoord;
          return o;
        }

        half4 fragment fragmentMain( v2f in [[stage_in]], texture2d< half, access::sample > tex [[texture(0)]] )
        {
          constexpr sampler s( address::repeat, filter::nearest );
          half3 texel = tex.sample( s, in.texcoord ).rgb;
          return half4( texel, 1.0 );
        }
    )";

  NS::Error* pError = nullptr;
  MTL::Library* pLibrary =
      device_->newLibrary(NS::String::string(shaderSrc, UTF8StringEncoding), nullptr, &pError);
  if (!pLibrary) {
    __builtin_printf("%s", pError->localizedDescription()->utf8String());
    assert(false);
  }

  // Set that the vertex function is called `vertexMain`.
  MTL::Function* pVertexFn = pLibrary->newFunction(NS::String::string("vertexMain", UTF8StringEncoding));
  // Set that the fragment function is called `fragmentMain`.
  MTL::Function* pFragFn = pLibrary->newFunction(NS::String::string("fragmentMain", UTF8StringEncoding));

  MTL::RenderPipelineDescriptor* pDesc = MTL::RenderPipelineDescriptor::alloc()->init();
  pDesc->setVertexFunction(pVertexFn);
  pDesc->setFragmentFunction(pFragFn);
  pDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

  pipeline_state_ = device_->newRenderPipelineState(pDesc, &pError);
  if (!pipeline_state_) {
    __builtin_printf("%s", pError->localizedDescription()->utf8String());
    assert(false);
  }

  pVertexFn->release();
  pFragFn->release();
  pDesc->release();
  pLibrary->release();
}

void Renderer::buildDepthStencilStates() {
  // MTL::DepthStencilDescriptor* pDsDesc = MTL::DepthStencilDescriptor::alloc()->init();
  // pDsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
  // pDsDesc->setDepthWriteEnabled(true);
  //
  // _pDepthStencilState = _pDevice->newDepthStencilState(pDsDesc);
  //
  // pDsDesc->release();
}

void Renderer::buildTextures() {}

void Renderer::buildBuffers() {
  float end = 1.0;

  simd::float3 positions[] = {
      {-end, +end, 0.0f},
      {-end, -end, 0.0f},
      {+end, -end, 0.0f},
      {+end, +end, 0.0f},
  };

  shader_types::VertexData verts[] = {
      {positions[0], {0.f, 0.f}},
      {positions[1], {0.f, 1.f}},
      {positions[2], {1.f, 1.f}},
      {positions[3], {1.f, 0.f}},
  };

  uint16_t indices[] = {0, 1, 2, 0, 2, 3};

  auto vertexDataSize = sizeof(verts);
  auto indexDataSize  = sizeof(indices);

  // Set vertex data buffer.
  _pVertexDataBuffer = device_->newBuffer(vertexDataSize, MTL::ResourceStorageModeManaged);
  memcpy(_pVertexDataBuffer->contents(), verts, vertexDataSize);
  _pVertexDataBuffer->didModifyRange(NS::Range::Make(0, _pVertexDataBuffer->length()));

  // Set index buffer data.
  _pIndexBuffer = device_->newBuffer(6 * sizeof(uint16_t), MTL::ResourceStorageModeManaged);
  memcpy(_pIndexBuffer->contents(), indices, indexDataSize * sizeof(uint16_t));
  _pIndexBuffer->didModifyRange(NS::Range::Make(0, _pIndexBuffer->length()));
}

void Renderer::draw(MTK::View* pView) {
  using simd::float3;
  using simd::float4;
  using simd::float4x4;

  NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

  MTL::CommandBuffer* pCmd        = command_queue_->commandBuffer();
  MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder(pRpd);

  pEnc->setRenderPipelineState(pipeline_state_);

  // Pass in data as 0th arg.
  pEnc->setVertexBuffer(_pVertexDataBuffer, 0, 0);

  // Update texture.
  texture_bitmap_.Update();
  // Use the texture bitmap.
  pEnc->setFragmentTexture(texture_bitmap_.GetTexture(), /* index */ 0);

  pEnc->drawIndexedPrimitives(
      MTL::PrimitiveType::PrimitiveTypeTriangle, 6, MTL::IndexTypeUInt16, _pIndexBuffer, 0);

  pEnc->endEncoding();
  pCmd->presentDrawable(pView->currentDrawable());
  pCmd->commit();

  pPool->release();

  // Input checkpoint.
  input::Input::Checkpoint();
}


}  // namespace pixelengine::app