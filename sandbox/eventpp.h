#pragma once
#include <cstdint>

#include <eventpp/eventdispatcher.h>

void EventppTest();

namespace evpp {

// Core
struct Event {
  uint32_t type;
  virtual ~Event() = default;
};

struct EventPolicies {
  static uint32_t getEvent(const Event& e, bool) { return e.type; }
};

using Dispatcher = eventpp::EventDispatcher<uint32_t, void(const Event&), EventPolicies>;

// User events
struct Packet : Event {
  uint64_t size;
  uint64_t time;
};

struct Connect : Event {
  uint64_t from;
  uint64_t to;
  uint64_t time;
};

struct More : Event {
  uint64_t data[16];
};

void PerfTest();

}  // namespace evpp