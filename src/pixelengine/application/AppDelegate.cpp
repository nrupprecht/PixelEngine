//
// Created by Nathaniel Rupprecht on 9/7/24.
//

#include "pixelengine/application/AppDelegate.h"
// Other files.

// Some constants.
static constexpr size_t kInstanceRows      = 3;
static constexpr size_t kInstanceColumns   = 3;
static constexpr size_t kInstanceDepth     = 3;
static constexpr size_t kNumInstances      = (kInstanceRows * kInstanceColumns * kInstanceDepth);


// TODO: Move to another place.
namespace shader_types {

struct VertexData {
  simd::float3 position;
  simd::float3 normal;
  simd::float2 texcoord;
};

struct InstanceData {
  simd::float4x4 instanceTransform;
  simd::float3x3 instanceNormalTransform;
  simd::float4 instanceColor;
};

struct CameraData {
  simd::float4x4 perspectiveTransform;
  simd::float4x4 worldTransform;
  simd::float3x3 worldNormalTransform;
};

}  // namespace shader_types

namespace pixelengine::app {

MyAppDelegate::~MyAppDelegate() {
  _pMtkView->release();
  _pWindow->release();
  _pDevice->release();
}

NS::Menu* MyAppDelegate::createMenuBar() {
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

void MyAppDelegate::applicationWillFinishLaunching(NS::Notification* pNotification) {
  NS::Menu* pMenu       = createMenuBar();
  NS::Application* pApp = reinterpret_cast<NS::Application*>(pNotification->object());
  pApp->setMainMenu(pMenu);
  pApp->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
}

void MyAppDelegate::applicationDidFinishLaunching(NS::Notification* pNotification) {
  CGRect frame = (CGRect) {{100.0, 100.0}, {1024.0, 1024.0}};

  _pWindow = NS::Window::alloc()->init(
      frame, NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled, NS::BackingStoreBuffered, false);

  _pDevice = MTL::CreateSystemDefaultDevice();

  _pMtkView = MTK::View::alloc()->init(frame, _pDevice);
  _pMtkView->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
  _pMtkView->setClearColor(MTL::ClearColor::Make(0.1, 0.1, 0.1, 1.0));
  _pMtkView->setDepthStencilPixelFormat(MTL::PixelFormat::PixelFormatDepth16Unorm);
  _pMtkView->setClearDepth(1.0f);

  _pViewDelegate = std::make_unique<MyMTKViewDelegate>(_pDevice, &world_);
  _pMtkView->setDelegate(_pViewDelegate.get());

  _pWindow->setContentView(_pMtkView);
  _pWindow->setTitle(NS::String::string(window_title_.c_str(), NS::StringEncoding::UTF8StringEncoding));

  _pWindow->makeKeyAndOrderFront(nullptr);

  auto pApp = reinterpret_cast<NS::Application*>(pNotification->object());
  pApp->activateIgnoringOtherApps(true);
}

bool MyAppDelegate::applicationShouldTerminateAfterLastWindowClosed(NS::Application* pSender) {
  return true;
}

MyMTKViewDelegate::MyMTKViewDelegate(MTL::Device* pDevice, world::World* world)
    : MTK::ViewDelegate()
    , _pRenderer(new Renderer(pDevice, world))
    , world_(world) {}

MyMTKViewDelegate::~MyMTKViewDelegate() {
  delete _pRenderer;
}

void MyMTKViewDelegate::drawInMTKView(MTK::View* pView) {
  assert(world_);
  world_->Update(1. / 60.);
  auto&& texture = _pRenderer->GetTexture();

  // TODO: This logic being here is temporary.
  // Update pixels to render the world.
  for (auto j = 0u; j < texture.GetHeight(); ++j) {
    for (auto i = 0u; i < texture.GetWidth(); ++i) {
      auto x        = i;
      auto y        = texture.GetHeight() - 1 - j;
      auto&& square = world_->GetSquare(i, j);
      texture.SetPixel(x, y, square.color);
    }
  }
  // TODO: End temp behavior.

  _pRenderer->draw(pView);
}


const int Renderer::kMaxFramesInFlight = 3;

Renderer::Renderer(MTL::Device* pDevice, world::World* world) : _pDevice(pDevice->retain()), _angle(0.f), _frame(0), world_(world) {
  _pCommandQueue = _pDevice->newCommandQueue();
  buildShaders();
  buildDepthStencilStates();
  buildTextures();
  buildBuffers();

  _semaphore = dispatch_semaphore_create(Renderer::kMaxFramesInFlight);
}

Renderer::~Renderer() {
  _pShaderLibrary->release();
  _pDepthStencilState->release();
  _pVertexDataBuffer->release();
  for (int i = 0; i < kMaxFramesInFlight; ++i) {
    _pInstanceDataBuffer[i]->release();
  }
  for (int i = 0; i < kMaxFramesInFlight; ++i) {
    _pCameraDataBuffer[i]->release();
  }
  _pIndexBuffer->release();
  _pPSO->release();
  _pCommandQueue->release();
  _pDevice->release();
}

void Renderer::buildShaders() {
  using NS::StringEncoding::UTF8StringEncoding;

  const char* shaderSrc = R"(
        #include <metal_stdlib>
        using namespace metal;

        struct v2f
        {
            float4 position [[position]];
            float3 normal;
            half3 color;
            float2 texcoord;
        };

        struct VertexData
        {
            float3 position;
            float3 normal;
            float2 texcoord;
        };

        struct InstanceData
        {
            float4x4 instanceTransform;
            float3x3 instanceNormalTransform;
            float4 instanceColor;
        };

        struct CameraData
        {
            float4x4 perspectiveTransform;
            float4x4 worldTransform;
            float3x3 worldNormalTransform;
        };

        v2f vertex vertexMain( device const VertexData* vertexData [[buffer(0)]],
                               device const InstanceData* instanceData [[buffer(1)]],
                               device const CameraData& cameraData [[buffer(2)]],
                               uint vertexId [[vertex_id]],
                               uint instanceId [[instance_id]] )
        {
            v2f o;

            const device VertexData& vd = vertexData[ vertexId ];
            float4 pos = float4( vd.position, 1.0 );
            pos = instanceData[ instanceId ].instanceTransform * pos;
            pos = cameraData.perspectiveTransform * cameraData.worldTransform * pos;
            o.position = pos;

            float3 normal = instanceData[ instanceId ].instanceNormalTransform * vd.normal;
            normal = cameraData.worldNormalTransform * normal;
            o.normal = normal;

            o.texcoord = vd.texcoord.xy;

            o.color = half3( instanceData[ instanceId ].instanceColor.rgb );
            return o;
        }

        half4 fragment fragmentMain( v2f in [[stage_in]], texture2d< half, access::sample > tex [[texture(0)]] )
        {
            constexpr sampler s( address::repeat, filter::linear );
            half3 texel = tex.sample( s, in.texcoord ).rgb;

            // assume light coming from (front-top-right)
            float3 l = normalize(float3( 1.0, 1.0, 0.8 ));
            float3 n = normalize( in.normal );

            half ndotl = half( saturate( dot( n, l ) ) );

            half3 illum = (in.color * texel * 0.1) + (in.color * texel * ndotl);
            return half4( illum, 1.0 );
        }
    )";

  NS::Error* pError = nullptr;
  MTL::Library* pLibrary =
      _pDevice->newLibrary(NS::String::string(shaderSrc, UTF8StringEncoding), nullptr, &pError);
  if (!pLibrary) {
    __builtin_printf("%s", pError->localizedDescription()->utf8String());
    assert(false);
  }

  MTL::Function* pVertexFn = pLibrary->newFunction(NS::String::string("vertexMain", UTF8StringEncoding));
  MTL::Function* pFragFn   = pLibrary->newFunction(NS::String::string("fragmentMain", UTF8StringEncoding));

  MTL::RenderPipelineDescriptor* pDesc = MTL::RenderPipelineDescriptor::alloc()->init();
  pDesc->setVertexFunction(pVertexFn);
  pDesc->setFragmentFunction(pFragFn);
  pDesc->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);
  pDesc->setDepthAttachmentPixelFormat(MTL::PixelFormat::PixelFormatDepth16Unorm);

  _pPSO = _pDevice->newRenderPipelineState(pDesc, &pError);
  if (!_pPSO) {
    __builtin_printf("%s", pError->localizedDescription()->utf8String());
    assert(false);
  }

  pVertexFn->release();
  pFragFn->release();
  pDesc->release();
  _pShaderLibrary = pLibrary;
}

void Renderer::buildDepthStencilStates() {
  MTL::DepthStencilDescriptor* pDsDesc = MTL::DepthStencilDescriptor::alloc()->init();
  pDsDesc->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
  pDsDesc->setDepthWriteEnabled(true);

  _pDepthStencilState = _pDevice->newDepthStencilState(pDsDesc);

  pDsDesc->release();
}

void Renderer::buildTextures() {
  const uint32_t tw = world_->GetWidth();
  const uint32_t th = world_->GetHeight();
  texture_bitmap_.Initialize(tw, th, _pDevice);
}

void Renderer::buildBuffers() {
  using simd::float2;
  using simd::float3;

  const float s = 0.5f;

  shader_types::VertexData verts[] = {
      //                                         Texture
      //   Positions           Normals         Coordinates
      {{-s, -s, +s}, {0.f, 0.f, 1.f}, {0.f, 1.f}},  {{+s, -s, +s}, {0.f, 0.f, 1.f}, {1.f, 1.f}},
      {{+s, +s, +s}, {0.f, 0.f, 1.f}, {1.f, 0.f}},  {{-s, +s, +s}, {0.f, 0.f, 1.f}, {0.f, 0.f}},

      {{+s, -s, +s}, {1.f, 0.f, 0.f}, {0.f, 1.f}},  {{+s, -s, -s}, {1.f, 0.f, 0.f}, {1.f, 1.f}},
      {{+s, +s, -s}, {1.f, 0.f, 0.f}, {1.f, 0.f}},  {{+s, +s, +s}, {1.f, 0.f, 0.f}, {0.f, 0.f}},

      {{+s, -s, -s}, {0.f, 0.f, -1.f}, {0.f, 1.f}}, {{-s, -s, -s}, {0.f, 0.f, -1.f}, {1.f, 1.f}},
      {{-s, +s, -s}, {0.f, 0.f, -1.f}, {1.f, 0.f}}, {{+s, +s, -s}, {0.f, 0.f, -1.f}, {0.f, 0.f}},

      {{-s, -s, -s}, {-1.f, 0.f, 0.f}, {0.f, 1.f}}, {{-s, -s, +s}, {-1.f, 0.f, 0.f}, {1.f, 1.f}},
      {{-s, +s, +s}, {-1.f, 0.f, 0.f}, {1.f, 0.f}}, {{-s, +s, -s}, {-1.f, 0.f, 0.f}, {0.f, 0.f}},

      {{-s, +s, +s}, {0.f, 1.f, 0.f}, {0.f, 1.f}},  {{+s, +s, +s}, {0.f, 1.f, 0.f}, {1.f, 1.f}},
      {{+s, +s, -s}, {0.f, 1.f, 0.f}, {1.f, 0.f}},  {{-s, +s, -s}, {0.f, 1.f, 0.f}, {0.f, 0.f}},

      {{-s, -s, -s}, {0.f, -1.f, 0.f}, {0.f, 1.f}}, {{+s, -s, -s}, {0.f, -1.f, 0.f}, {1.f, 1.f}},
      {{+s, -s, +s}, {0.f, -1.f, 0.f}, {1.f, 0.f}}, {{-s, -s, +s}, {0.f, -1.f, 0.f}, {0.f, 0.f}}};

  uint16_t indices[] = {
      0,  1,  2,  2,  3,  0,  /* front */
      4,  5,  6,  6,  7,  4,  /* right */
      8,  9,  10, 10, 11, 8,  /* back */
      12, 13, 14, 14, 15, 12, /* left */
      16, 17, 18, 18, 19, 16, /* top */
      20, 21, 22, 22, 23, 20, /* bottom */
  };

  const size_t vertexDataSize = sizeof(verts);
  const size_t indexDataSize  = sizeof(indices);

  MTL::Buffer* pVertexBuffer = _pDevice->newBuffer(vertexDataSize, MTL::ResourceStorageModeManaged);
  MTL::Buffer* pIndexBuffer  = _pDevice->newBuffer(indexDataSize, MTL::ResourceStorageModeManaged);

  _pVertexDataBuffer = pVertexBuffer;
  _pIndexBuffer      = pIndexBuffer;

  memcpy(_pVertexDataBuffer->contents(), verts, vertexDataSize);
  memcpy(_pIndexBuffer->contents(), indices, indexDataSize);

  _pVertexDataBuffer->didModifyRange(NS::Range::Make(0, _pVertexDataBuffer->length()));
  _pIndexBuffer->didModifyRange(NS::Range::Make(0, _pIndexBuffer->length()));

  const size_t instanceDataSize = kMaxFramesInFlight * kNumInstances * sizeof(shader_types::InstanceData);
  for (size_t i = 0; i < kMaxFramesInFlight; ++i) {
    _pInstanceDataBuffer[i] = _pDevice->newBuffer(instanceDataSize, MTL::ResourceStorageModeManaged);
  }

  const size_t cameraDataSize = kMaxFramesInFlight * sizeof(shader_types::CameraData);
  for (size_t i = 0; i < kMaxFramesInFlight; ++i) {
    _pCameraDataBuffer[i] = _pDevice->newBuffer(cameraDataSize, MTL::ResourceStorageModeManaged);
  }
}

void Renderer::draw(MTK::View* pView) {
  using simd::float3;
  using simd::float4;
  using simd::float4x4;

  std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();

  // Difference between last_time_ and now.
  std::chrono::duration<double> elapsed = t0 - last_render_time_;
  // Convert to microseconds.
  last_frame_time_us_ = elapsed.count() * 1'000'000.0;
  last_render_time_   = t0;
  ++num_frames_;

  // std::cout << num_frames_ << ": FPS = " << 1'000'000.0 / last_frame_time_us_ << std::endl;

  NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

  _frame                           = (_frame + 1) % Renderer::kMaxFramesInFlight;
  MTL::Buffer* pInstanceDataBuffer = _pInstanceDataBuffer[_frame];

  MTL::CommandBuffer* pCmd = _pCommandQueue->commandBuffer();
  dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);
  Renderer* pRenderer = this;
  pCmd->addCompletedHandler(
      ^void(MTL::CommandBuffer* pCmd) { dispatch_semaphore_signal(pRenderer->_semaphore); });

  // TODO: Remove
  _angle += 0.002f;

  const float scl = 0.2f;
  shader_types::InstanceData* pInstanceData =
      reinterpret_cast<shader_types::InstanceData*>(pInstanceDataBuffer->contents());

  // Where to put the screen that is being rendered to.
  float3 objectPosition = {0.f, 0.f, -4.f};
  // TODO: Change
  // float3 objectPosition = {0.f, 0.f, -0.15f};

  float4x4 rt            = math::makeTranslate(objectPosition);
  float4x4 rr1           = math::makeYRotate(-_angle);
  float4x4 rr0           = math::makeXRotate(_angle * 0.5);
  float4x4 rtInv         = math::makeTranslate({-objectPosition.x, -objectPosition.y, -objectPosition.z});
  float4x4 fullObjectRot = rt * rr1 * rr0 * rtInv;

  size_t ix = 0;
  size_t iy = 0;
  size_t iz = 0;
  for (size_t i = 0; i < kNumInstances; ++i) {
    if (ix == kInstanceRows) {
      ix = 0;
      iy += 1;
    }
    if (iy == kInstanceRows) {
      iy = 0;
      iz += 1;
    }

    float4x4 scale = math::makeScale((float3) {scl, scl, scl});
    float4x4 zrot  = math::makeZRotate(_angle * sinf((float)ix));
    float4x4 yrot  = math::makeYRotate(_angle * cosf((float)iy));

    float x            = ((float)ix - (float)kInstanceRows / 2.f) * (2.f * scl) + scl;
    float y            = ((float)iy - (float)kInstanceColumns / 2.f) * (2.f * scl) + scl;
    float z            = ((float)iz - (float)kInstanceDepth / 2.f) * (2.f * scl);
    float4x4 translate = math::makeTranslate(math::add(objectPosition, {x, y, z}));

    pInstanceData[i].instanceTransform       = fullObjectRot * translate * yrot * zrot * scale;
    pInstanceData[i].instanceNormalTransform = math::discardTranslation(pInstanceData[i].instanceTransform);

    float iDivNumInstances         = i / (float)kNumInstances;
    float r                        = iDivNumInstances;
    float g                        = 1.0f - r;
    float b                        = sinf(M_PI * 2.0f * iDivNumInstances);
    pInstanceData[i].instanceColor = (float4) {// r, g, b, 1.0f
                                               1.0,
                                               1.0,
                                               1.0,
                                               1.0};

    ix += 1;
  }
  pInstanceDataBuffer->didModifyRange(NS::Range::Make(0, pInstanceDataBuffer->length()));

  // Update camera state:

  MTL::Buffer* pCameraDataBuffer = _pCameraDataBuffer[_frame];
  shader_types::CameraData* pCameraData =
      reinterpret_cast<shader_types::CameraData*>(pCameraDataBuffer->contents());
  pCameraData->perspectiveTransform = math::makePerspective(45.f * M_PI / 180.f, 1.f, 0.03f, 500.0f);
  pCameraData->worldTransform       = math::makeIdentity();
  pCameraData->worldNormalTransform = math::discardTranslation(pCameraData->worldTransform);
  pCameraDataBuffer->didModifyRange(NS::Range::Make(0, sizeof(shader_types::CameraData)));

  // Begin render pass:

  MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
  MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder(pRpd);

  pEnc->setRenderPipelineState(_pPSO);
  pEnc->setDepthStencilState(_pDepthStencilState);

  pEnc->setVertexBuffer(_pVertexDataBuffer, /* offset */ 0, /* index */ 0);
  pEnc->setVertexBuffer(pInstanceDataBuffer, /* offset */ 0, /* index */ 1);
  pEnc->setVertexBuffer(pCameraDataBuffer, /* offset */ 0, /* index */ 2);

  // Update texture.
  texture_bitmap_.Update();
  // End update texture.

  // Use the texture bitmap.
  pEnc->setFragmentTexture(texture_bitmap_.GetTexture(), /* index */ 0);

  pEnc->setCullMode(MTL::CullModeBack);
  pEnc->setFrontFacingWinding(MTL::Winding::WindingCounterClockwise);

  pEnc->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                              6 * 6,
                              MTL::IndexType::IndexTypeUInt16,
                              _pIndexBuffer,
                              0,
                              kNumInstances);

  pEnc->endEncoding();
  pCmd->presentDrawable(pView->currentDrawable());
  pCmd->commit();

  pPool->release();
}


}  // namespace pixelengine::app