//
// Created by Nathaniel Rupprecht on 9/29/24.
//

#pragma once

#include <list>

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

#include "pixelengine/utility/Vec2.h"
#include "pixelengine/utility/WindowContext.h"


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
  Node()          = default;
  virtual ~Node() = default;

  //! \brief Set an identifying name for the node. Purely for debuging purposes.
  void SetName(std::string name) { name_ = std::move(name); }

  //! \brief Get the name of the node.
  [[nodiscard]] const std::string& GetName() const { return name_; }

  //! \brief Add a child of the node. Adds as the child with the least precedence.
  void AddChild(std::unique_ptr<Node> child) {
    if (child->parent_) {
      child->parent_->QueueRemoveChild(child.get());
    }
    queued_children_.push_back(std::move(child));
  }

  //! \brief Queue a node to be removed as a child of this node. If the node is not a child, does nothing.
  void QueueRemoveChild(Node* child) {
    auto it = std::ranges::find_if(children_, [child](const auto& c) { return c.get() == child; });
    if (it != children_.end()) {
      (*it)->queued_for_deletion_ = true;
    }
  }

  Node& AsNode() { return *this; }

private:
  void addedBy(Node* parent) {
    parent_ = parent;
    _onAddedBy(parent);
  }

  void removeQueuedChildren() {
    children_.remove_if([this](const auto& child) {
      if (child->queued_for_deletion_) {
        _childLeaving(child.get());
        child->_onLeavingFrom(this);
      }
      return child->queued_for_deletion_;
    });
  }

  void addQueuedChildren() {
    auto former_size = children_.size();
    children_.splice(children_.end(), std::move(queued_children_));
    auto count = 0;
    for (auto it = children_.rbegin(); former_size < children_.size() - count; ++it, ++count) {
      // Callback to the node that it was added to the parent
      (*it)->addedBy(this);
      _childEntering(it->get());
    }

    for (auto& child : children_) {
      child->addQueuedChildren();
    }
  }

  void interactWithWorld(world::World* world) {
    _interactWithWorld(world);

    // Potentially pass a different world to lower levels. This should done e.g. if this node is itself a
    // world.
    auto world_to_pass = _setWorld(world);
    for (auto& child : children_) {
      child->interactWithWorld(world_to_pass);
    }
  }

  void prePhysics(float dt) {
    _prePhysics(dt);
    for (auto& child : children_) {
      child->prePhysics(dt);
    }
  }

  void updatePhysics(float dt, world::World* world) {
    _interactWithWorld(world);
    _updatePhysics(dt);

    // Potentially pass a different world to lower levels. This should done e.g. if this node is itself a
    // world.
    auto world_to_pass = _setWorld(world);
    for (auto& child : children_) {
      child->updatePhysics(dt, world_to_pass);
    }
  }

  void update(float dt) {
    _update(dt);
    for (auto& child : children_) {
      child->update(dt);
    }
  }

  void draw(MTL::RenderCommandEncoder* render_command_encoder,
            application::WindowContext* context,
            Vec2 parent_offset) {
    _draw(render_command_encoder, context, parent_offset);
    auto context_to_pass   = _chooseWindowContext(context);
    auto additional_offset = _additionalOffset();
    for (auto& child : children_) {
      child->draw(render_command_encoder, context_to_pass, parent_offset + additional_offset);
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
  virtual void _interactWithWorld([[maybe_unused]] world::World* world) {}

  virtual void _prePhysics([[maybe_unused]] float dt) {}
  virtual void _updatePhysics([[maybe_unused]] float dt) {}
  virtual void _update([[maybe_unused]] float dt) {}

  virtual void _draw([[maybe_unused]] MTL::RenderCommandEncoder* render_command_encoder,
                     [[maybe_unused]] application::WindowContext* context,
                     [[maybe_unused]] Vec2 parent_offset) {}

  //! \brief Called right after the node is added to the Tree, and before the parent has _childEntering
  //!        called.
  virtual void _onAddedBy([[maybe_unused]] Node* parent) {}
  //! \brief Called right after the child node enters the Tree.
  virtual void _childEntering([[maybe_unused]] Node* child) {}

  //! \brief Called right before the child node exits the Tree.
  virtual void _childLeaving([[maybe_unused]] Node* child) {}
  //! \brief Called right before the node is removed from the Tree, and after the parent has _childLeaving
  virtual void _onLeavingFrom([[maybe_unused]] Node* parent) {}

  // ===========================================================================
  //  Special overrideable functions.
  // ===========================================================================

  //! \brief Set the world that should be passed to children of this node.
  [[nodiscard]] virtual world::World* _setWorld(world::World* world) { return world; }

  //! \brief Set the window context that should be passed to children of this node.
  [[nodiscard]] virtual application::WindowContext* _chooseWindowContext(
      application::WindowContext* context) {
    return context;
  }

  [[nodiscard]] virtual Vec2 _additionalOffset() const { return {}; }

  // ===========================================================================
  //  Protected member variables.
  // ===========================================================================

private:
  std::string name_ = "<unnamed>";

  Node* parent_ {};

  bool queued_for_deletion_ {false};

  std::list<std::unique_ptr<Node>> queued_children_;

  std::list<std::unique_ptr<Node>> children_;
};

}  // namespace pixelengine