//
// Created by Nathaniel Rupprecht on 9/29/24.
//

#pragma once

#include "pixelengine/physics/PhysicsBody.h"

namespace minesandmagic {

using pixelengine::Vec2;

class Player : public pixelengine::physics::PhysicsBody {
public:
  Player(Vec2 position, float width, float height, Vec2 velocity = {})
    : PhysicsBody(position, width, height, velocity) {}

private:
  void _update(float dt) override;
};

}