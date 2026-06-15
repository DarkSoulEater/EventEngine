#include <benchmark/benchmark.h>
#include <event/event.h>
#include <event/handler.h>
#include <eventpp/eventdispatcher.h>

// Event
namespace event::test {
struct StubEvent : public EventBase<StubEvent> {
  int a = 0;
};

struct PacketEvent : public EventBase<PacketEvent> {
  uint64_t size;
  uint64_t time;

  PacketEvent(uint64_t size, uint64_t time) : size(size), time(time) {}
};

static void BM_C_DispatchSingleHandlersEvent(benchmark::State& state) {
  uint64_t sum = 0;
  syncSubscribe<PacketEvent>([&](const PacketEvent& e) { sum += e.size * e.time; });

  for (auto _ : state) {
    event::dispatch<PacketEvent>(1ull, 2ull);
  }

  benchmark::DoNotOptimize(sum);
  detail::clearHandlers();
}
}  // namespace event::test

// Eventpp
namespace evpp {
struct Event {
  uint32_t type{0};
  virtual ~Event() = default;

  explicit Event(uint32_t type) : type(type) {}
};

struct EventPolicies {
  static uint32_t getEvent(const Event& e) { return e.type; }
};

using Dispatcher = eventpp::EventDispatcher<uint32_t, void(const Event&), EventPolicies>;

// User events
struct Packet : Event {
  uint64_t size;
  uint64_t time;

  Packet(uint64_t size, uint64_t time) : Event(1), size(size), time(time) {}
};

struct Connect : Event {
  uint64_t from;
  uint64_t to;
  uint64_t time;
};

struct More : Event {
  uint64_t data[16];
};

static void BM_C_DispatchSingleHandlersEvent(benchmark::State& state) {
  uint64_t   sum = 0;
  Dispatcher dispatcher;
  dispatcher.appendListener(1, [&](const Event& event) {
    const auto& e = reinterpret_cast<const Packet&>(event);
    sum += e.size * e.time;
  });

  for (auto _ : state) {
    dispatcher.dispatch(Packet{1, 2});
  }

  benchmark::DoNotOptimize(sum);
}
}  // namespace evpp

BENCHMARK(event::test::BM_C_DispatchSingleHandlersEvent);
BENCHMARK(evpp::BM_C_DispatchSingleHandlersEvent);