#pragma once

#include "pixelengine/physics/PhysicsBody.h"

namespace pixelengine::physics {

//! \brief All the physics bodies in an interaction are able to interact with one another.
//!        The InteractionSystem is responsible for implementing the algorith for computing
//!        the forces or interactions between the bodies.
class InteractionSystem {
public:
  void AddMember(PhysicsBody* body) {
    queued_members_.push_back(body);
    body->_onAddedToInteractionSystem(this);
  }

  virtual void QueueRemoveMember(PhysicsBody* body) {
    queued_for_removal_.push_back(body);
    body->_onRemovedFromInteractionSystem(this);
  }

private:
  void _updatePhysics(float dt) { computeInteractions(dt); }

  virtual void removeQueuedMembers() = 0;
  virtual void addQueuedMembers()    = 0;

  virtual void computeInteractions(float dt) = 0;

  std::vector<PhysicsBody*> queued_members_;
  std::vector<PhysicsBody*> queued_for_removal_;
};

}  // namespace pixelengine::physics