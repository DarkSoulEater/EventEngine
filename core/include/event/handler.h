#ifndef CORE_EVENT_HANDLER_H
#define CORE_EVENT_HANDLER_H

#include <functional>

class Handler {
 public:
  template <typename EventType>
  explicit Handler(std::function<void(const EventType&)> callback) {
    callback_data_ = new auto(std::move(callback));

    invoke_ = [](const void* event, const void* callback_data) {
      auto& callback = *static_cast<const std::function<void(const EventType&)>*>(callback_data);
      callback(*static_cast<const EventType*>(event));
    };

    destroy_ = [](void* data) { delete static_cast<std::function<void(const EventType&)>*>(data); };
  }

  Handler(const Handler& other)      = delete;
  Handler& operator=(const Handler&) = delete;

  Handler(Handler&& other) noexcept
      : invoke_(other.invoke_), destroy_(other.destroy_), callback_data_(other.callback_data_) {
    other.destroy_       = nullptr;
    other.invoke_        = nullptr;
    other.callback_data_ = nullptr;
  }

  ~Handler() {
    if (destroy_) {
      destroy_(callback_data_);
    }
  }

  void handle(const void* event) { invoke_(event, callback_data_); }

 private:
  using InvokeFunction  = void (*)(const void*, const void*);
  using DestroyFunction = void (*)(void*);

  InvokeFunction  invoke_{nullptr};
  DestroyFunction destroy_{nullptr};
  void*           callback_data_{nullptr};
};

#endif  // CORE_EVENT_HANDLER_H