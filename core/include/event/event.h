#pragma once

#include <atomic>
#include <chrono>

namespace event {

class IdGenerator {
public:
  inline static size_t nextId() {
    static std::atomic<uint64_t> id_generator_{0};
    return id_generator_++;
  }
};

template <typename Derived> class EventBase : private IdGenerator {
public:
  typedef std::chrono::time_point<std::chrono::system_clock> DateTime;

  explicit EventBase(
      const DateTime &timestamp = std::chrono::system_clock::now())
      : timestamp(timestamp) {}

  ~EventBase() = default;

  inline static size_t getTypeId() {
    static const size_t type_id = nextId();
    return type_id;
  }

public:
  DateTime timestamp;
};

}; // namespace event