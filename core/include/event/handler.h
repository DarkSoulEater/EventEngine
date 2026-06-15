#ifndef CORE_EVENT_HANDLER_H
#define CORE_EVENT_HANDLER_H

#include <function2/function2.hpp>

namespace event {
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

namespace detail {
struct HandlerStorage {
  std::vector<std::vector<Handler>> sync;
  std::vector<std::vector<Handler>> async;
};

inline auto& handlerStorage() {
  static HandlerStorage storage;
  return storage;
}

inline void clearHandlers() {
  auto& handlers = handlerStorage();

  handlers.sync.clear();
  handlers.async.clear();

  handlers.sync.shrink_to_fit();
  handlers.async.shrink_to_fit();
}
}  // namespace detail

template <typename EventType, typename Callback>
void syncSubscribe(Callback&& callback) {
  auto& storage = detail::handlerStorage();
  auto& sync    = storage.sync;

  const size_t id = EventType::getTypeId();

  if (sync.size() <= id) {
    sync.resize(id + 1);
  }

  sync[id].emplace_back(std::type_identity<EventType>{}, std::forward<Callback>(callback));
}

template <typename EventType, typename Callback>
void asyncSubscribe(Callback&& callback) {
  auto& storage = detail::handlerStorage();
  auto& async   = storage.async;

  constexpr size_t id = EventType::getTypeId();

  if (async.size() <= id) {
    async.resize(id + 1);
  }

  async[id].emplace_back(std::forward<Callback>(callback));
}

}  // namespace event

#endif  // CORE_EVENT_HANDLER_H