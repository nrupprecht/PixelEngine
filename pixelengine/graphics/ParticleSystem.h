//
// Created by Nathaniel Rupprecht on 9/29/24.
//

#pragma once

#include "pixelengine/graphics/Color.h"
#include "pixelengine/utility/Utility.h"
#include "pixelengine/graphics/ShaderProgram.h"
#include "pixelengine/graphics/RectangularDrawable.h"

namespace pixelengine::graphics {

struct Particle {
  Vec2 position;
  Vec2 velocity;
  float life;
  Color color;
};

class ParticleSystem {
public:
  ParticleSystem(ShaderProgram* shader_program,
              std::unique_ptr<TextureContainer> texture,
              float lifetime,
              MTL::Device* device)
    : lifetime_(lifetime), drawable_(shader_program, std::move(texture), device) {}

  void Draw(MTL::RenderCommandEncoder* cmd_encoder) {
    for (auto& particle : particles_) {
      drawable_.SetPosition(particle.position.x, particle.position.y);
      // drawable_.GetTextureBitmap().SetPixel(0, 0, particle.color);
      drawable_.Draw(cmd_encoder);
    }
  }

  void Update(float dt) {
    update(dt);

    // Logic for creating more particles.

  }

private:
  float lifetime_{};

  void emit(const Vec2& position, const Vec2& velocity, const Color& color) {
    particles_.emplace_back(position, velocity, 0.f, color);
  }

  void update(float dt) {
    while (!particles_.empty() && lifetime_ + dt < particles_.front().life) {
      particles_.pop_front();
    }
    for (auto& particle : particles_) {
      particle.position += particle.velocity * dt;
      particle.life += dt;
    }
  }

  RectangularDrawable drawable_;

  std::deque<Particle> particles_;
};

}  // namespace pixelengine::graphics