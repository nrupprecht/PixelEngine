//
// Created by Nathaniel Rupprecht on 9/29/24.
//

#include "minesandmagic/Player.h"
// Other files.
#include "pixelengine/input/Input.h"

namespace minesandmagic {

void Player::_update(float dt) {
  using namespace pixelengine::input;

  Node::_update(dt);

  bool moving_x = false;
  if (Input::IsPressed('D')) {
    applyForceX(+10.);
    moving_x = true;
  }
  if (Input::IsPressed('A')) {
    applyForceX(-10.);
    moving_x = true;
  }

  // Standing still.
  if (!moving_x && GetState().blocked_down) {
    setVelocityX(0.f);
  }

  // Jump.
  if (Input::IsPressed('W') && GetState().blocked_down /* Standing */) {
    addVelocityY(+4.f);
  }

  LOG_SEV(Debug) << "Player position: " << GetPosition().x << ", " << GetPosition().y << ", blocking: ["
                 << GetState() << "].";
}

}  // namespace minesandmagic