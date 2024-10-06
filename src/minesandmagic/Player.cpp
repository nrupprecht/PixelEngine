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

  clearVelocity();
  if (Input::IsPressed('D')) {
    addVelocityX(10.);
  }
  if (Input::IsPressed('A')) {
    addVelocityX(-10.);
  }

  // Temporary.
  if (Input::IsPressed('W')) {
    addVelocityY(10.);
  }
  if (Input::IsPressed('S')) {
    addVelocityY(-10.);
  }

  LOG_SEV(Debug) << "Player position: " << GetPosition().x << ", " << GetPosition().y << ".";
}

}  // namespace minesandmagic