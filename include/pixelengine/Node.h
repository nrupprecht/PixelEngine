//
// Created by Nathaniel Rupprecht on 9/29/24.
//

#pragma once

#include <list>
#include <memory>
#include <AppKit/AppKit.hpp>
#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>


namespace pixelengine {

namespace app {
class Game;
}  // namespace app

namespace world {
class World;
}  // namespace world

//! \brief Node in the game.
//!
//! removeQueuedChildren ---> addQueuedChildren ---> prePhysics ---> updatePhysics ---> update
class Node {
  // Game is a friend so it can call the various update functions.
  friend class app::Game;

public:
  virtual ~Node() = default;

  void AddChild(std::unique_ptr<Node> child) {
    if (child->parent_) {
      child->parent_->QueueRemoveChild(child.get());
    }
    children_.push_back(std::move(child));
    children_.back()->addedBy(this);
  }

  void QueueRemoveChild(Node* child) {
    auto it = std::ranges::find_if(children_, [child](const auto& c) { return c.get() == child; });
    if (it != children_.end()) {
      (*it)->queued_for_deletion_ = true;
    }
  }

  Node& AsNode() {
    return *this;
  }

private:
  void addedBy(Node* parent) {
    parent_ = parent;
    _onAddedBy(parent);
  }

  void removeQueuedChildren() {
    children_.remove_if([](const auto& child) { return child->queued_for_deletion_; });
  }

  void addQueuedChildren() {
    children_.splice(children_.end(), std::move(queued_children_));
    for (auto& child : children_) {
      child->addQueuedChildren();
    }
  }

  void interactWithWorld(world::World& world) {
    _interactWithWorld(world);
    for (auto& child : children_) {
      child->interactWithWorld(world);
    }
  }

  void prePhysics(float dt) {
    _prePhysics(dt);
    for (auto& child : children_) {
      child->prePhysics(dt);
    }
  }

  void updatePhysics(float dt) {
    _updatePhysics(dt);
    for (auto& child : children_) {
      child->updatePhysics(dt);
    }
  }

  void update(float dt) {
    _update(dt);
    for (auto& child : children_) {
      child->update(dt);
    }
  }

  //! \brief Transitively release all children, depth first.
  void releaseChildren() {
    for (auto& child : children_) {
      child->releaseChildren();
    }
    children_.clear();
  }

protected:
  virtual void _interactWithWorld(world::World& world) {}
  virtual void _prePhysics(float dt) {}
  virtual void _updatePhysics(float dt) {}
  virtual void _update(float dt) {}

  virtual void _draw(MTL::RenderCommandEncoder* render_command_encoder) {}

  virtual void _onAddedBy(Node* parent) {}

private:
  Node* parent_ {};

  bool queued_for_deletion_ {false};

  std::list<std::unique_ptr<Node>> queued_children_;

  std::list<std::unique_ptr<Node>> children_;
};

}  // namespace pixelengine