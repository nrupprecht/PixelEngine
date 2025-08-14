#pragma once

#include <functional>
#include <optional>
#include <tuple>
#include <utility>

#include "pixelengine/utility/Contracts.h"


namespace pixelengine {

// Forward declaration of Node class.
class Node;

//! \brief Function type for checking whether signals should be emitted, and what values they should pass to
//!        their listeners.
template<typename... Args_t>
using signal_check_t = std::function<std::optional<std::tuple<Args_t...>>()>;


//! \brief Signal represents events that other nodes (or things) can listen to.
template<typename... Args_t>
class Signal {
public:
  using Callback = std::function<void(Args_t...)>;

  void Connect(Node* node, Callback callback) {
    PIXEL_ASSERT(node, "Cannot connect signal to null node.");
    listeners_.emplace_back(node, std::move(callback));
  }

  //! \brief Emit the signal to all listeners.
  void Emit(Args_t... args) const {
    for (const auto& [node, callback] : listeners_) {
      callback(args...);
    }
  }

  void Disconnect(Node* node) {
    std::ranges::remove_if(listeners_, [node](const auto& pair) { return pair.first == node; });
  }

  void Clear() { listeners_.clear(); }

  std::size_t Size() const { return listeners_.size(); }
  bool Empty() const { return listeners_.empty(); }

private:
  std::vector<std::pair<Node*, Callback>> listeners_;
};

//! \brief Class used to register a node's signals and how how to check them.
class SignalEmitter {
public:
  template<typename... Args_t>
  SignalEmitter(Signal<Args_t...>* signal, signal_check_t<Args_t...> check);

  void CheckSignal();

private:
  class Impl;

  template<typename... Args_t>
  class ImplT;

  std::unique_ptr<Impl> impl_;
};

class SignalEmitter::Impl {
public:
  virtual ~Impl()            = default;
  virtual void checkSignal() = 0;
};

template<typename... Args_t>
class SignalEmitter::ImplT : public SignalEmitter::Impl {
public:
  using signal_t = Signal<Args_t...>;
  using check_t  = signal_check_t<Args_t...>;

  ImplT(const signal_t* signal, check_t check) : signal_(signal), check_signal_(std::move(check)) {}

  void checkSignal() override {
    if (signal_->Empty() || !check_signal_) {
      return;
    }
    if (auto result = check_signal_()) {
      std::apply([this](Args_t... args) { signal_->Emit(args...); }, *result);
    }
  }

private:
  const signal_t* signal_;
  check_t check_signal_;
};

template<typename... Args_t>
SignalEmitter::SignalEmitter(Signal<Args_t...>* signal, signal_check_t<Args_t...> check)
    : impl_(std::make_unique<ImplT<Args_t...>>(signal, std::move(check))) {}

inline void SignalEmitter::CheckSignal() {
  impl_->checkSignal();
}

}  // namespace pixelengine