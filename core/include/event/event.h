#pragma once

#include <atomic>
#include <chrono>

#include "exe/run.h"
#include "handler.h"

namespace event {

class IdGenerator {
 public:
  inline static size_t nextId() {
    static std::atomic<uint64_t> id_generator_{0};
    return id_generator_.fetch_add(1, std::memory_order::relaxed);
  }
};

template <typename Derived>
class EventBase : private IdGenerator {
 public:
  typedef std::chrono::time_point<std::chrono::system_clock> DateTime;

  explicit EventBase(const DateTime& timestamp = {}) : timestamp(timestamp) {}

  ~EventBase() = default;

  inline static size_t getTypeId() {
    static const size_t type_id = nextId();
    return type_id;
  }

 public:
  DateTime timestamp;
};

template <typename EventType, typename... Args>
void dispatch(Args&&... args) {
  const size_t id       = EventType::getTypeId();
  auto&        handlers = detail::handlerStorage();

  // Cpu-bound
  auto& sync = handlers.sync;
  if (id < sync.size()) {
    EventType event(std::forward<Args>(args)...);
    for (auto& handler : sync[id]) {
      handler.handle(&event);
    }
  }

  // IO-bound
  auto& async = handlers.async;
  if (id < async.size() && !async[id].empty()) {
    exe::Run([id, event = EventType(std::forward<Args>(args)...)] {
      auto& async = detail::handlerStorage().async;
      for (auto& handler : async[id]) {
        handler.handle(&event);
      }
    });
  }
}

};  // namespace event