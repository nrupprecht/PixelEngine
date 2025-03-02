//
// Created by Nathaniel Rupprecht on 9/29/24.
//

#pragma once

#include "pixelengine/physics/PhysicsBody.h"

namespace minesandmagic {

using pixelengine::Vec2;
using pixelengine::PVec2;

class Player : public pixelengine::physics::PhysicsBody {
public:
  Player(PVec2 position, unsigned width, unsigned height, Vec2 velocity = {})
    : PhysicsBody(position, width, height, velocity) {}

private:
  void _update(float dt) override;
};

}