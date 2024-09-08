//
// Created by Nathaniel Rupprecht on 9/6/24.
//

/*
 *
 * Copyright 2022 Apple Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



// Pixel engine.
#include <pixelengine/TextureBitmap.h>
#include <pixelengine/Utility.h>
#include <pixelengine/application/AppDelegate.h>

using namespace pixelengine;
using namespace pixelengine::world;


constexpr std::size_t WORLD_WIDTH  = 128;
constexpr std::size_t WORLD_HEIGHT = 128;

// Some sand colors.
// 284 127 125
// 204 171 114
// 200 168 113
// 179 149 100

constexpr Color SAND_COLORS[] = {
    Color(204, 171, 114),
    Color(200, 168, 113),
    Color(179, 149, 100),
    Color(179, 149, 100),
};

// A background color.
// 240 228 228

constexpr Color BACKGROUND = Color(240, 228, 228);


int main(int argc, char* argv[]) {
  NS::AutoreleasePool* pAutoreleasePool = NS::AutoreleasePool::alloc()->init();

  app::MyAppDelegate del(WORLD_WIDTH, WORLD_HEIGHT);

  // Set up.
  auto&& world = del.GetWorld();

  FallingPhysics falling{};
  LiquidPhysics liquid{};

  for (auto j = 0u; j < world.GetHeight(); ++j) {
    for (auto i = 0u; i < world.GetWidth(); ++i) {
      auto r = randf();
      if (r < 0.2) {
        if (i < world.GetWidth() / 2) {
          auto c = randf();
          Square sand_square(true, SAND_COLORS[static_cast<int>(4 * c)], &SAND, &falling);
          world.SetSquare(i, j, sand_square);
        }
        else {
          Square water_square(true, Color::FromFloats(0., 0., 1.), &WATER, &liquid);
          world.SetSquare(i, j, water_square);
        }
      }
      else {
        world.SetSquare(i, j, Square(false, BACKGROUND, &AIR, nullptr));
      }
    }
  }

  // End set up.

  NS::Application* application = NS::Application::sharedApplication();
  application->setDelegate(&del);
  application->run();
  pAutoreleasePool->release();

  return 0;
}



