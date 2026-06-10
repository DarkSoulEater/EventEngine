#ifndef CORE_EVENT_HANDLER_H
#define CORE_EVENT_HANDLER_H

#include <function2/function2.hpp>

class Handler {
 public:
  template <typename EventType, typename Callable>
  explicit Handler(std::type_identity<EventType>, Callable&& callback) {
    invoke_ = [cb = std::forward<Callable>(callback)](const void* event) {
      cb(*static_cast<const EventType*>(event));
    };
  }

  Handler(const Handler& other)      = delete;
  Handler& operator=(const Handler&) = delete;

  Handler(Handler&& other) noexcept = default;
  Handler& operator=(Handler&&)     = delete;

  ~Handler() = default;

  void handle(const void* event) { invoke_(event); }

 private:
  using InvokeFunction = fu2::unique_function<void(const void*)>;
  InvokeFunction invoke_{nullptr};
};

#endif  // CORE_EVENT_HANDLER_H