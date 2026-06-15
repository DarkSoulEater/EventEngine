#include <boost/signals2.hpp>
#include <sigslot/signal.hpp>

#include <benchmark/benchmark.h>
#include <event/event.h>
#include <event/handler.h>
#include <eventpp/eventdispatcher.h>

namespace {
struct alignas(std::hardware_destructive_interference_size) Counter {
  uint64_t count{0};
};

struct Counters {
  Counter counter[8];
};
}  // namespace

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

struct IdEvent : public EventBase<IdEvent> {
  uint64_t id;
  uint64_t data;

  IdEvent(uint64_t id, uint64_t data) : id(id), data(data) {}
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

static void BM_SCMP_DispatchSingleHandlersEvent(benchmark::State& state) {
  static Counters counters;
  if (state.thread_index() == 0) {
    syncSubscribe<IdEvent>([](const IdEvent& e) { counters.counter[e.id].count += e.data; });
  }

  const auto thread_id = state.thread_index();
  for (auto _ : state) {
    event::dispatch<IdEvent>(thread_id, 2ull);
  }

  benchmark::DoNotOptimize(counters);
  if (state.thread_index() == 0) {
    detail::clearHandlers();
  }
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

struct IdEvent : Event {
  uint64_t id;
  uint64_t data;

  IdEvent(uint64_t id, uint64_t data) : Event(2), id(id), data(data) {}
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

static void BM_SCMP_DispatchSingleHandlersEvent(benchmark::State& state) {
  static Counters   counters;
  static Dispatcher dispatcher;

  if (state.thread_index() == 0) {
    dispatcher.appendListener(2, [&](const Event& event) {
      const auto& e = reinterpret_cast<const IdEvent&>(event);
      counters.counter[e.id].count += e.data;
    });
  }

  unsigned long int thread_id = state.thread_index();
  for (auto _ : state) {
    dispatcher.dispatch(IdEvent{thread_id, 2});
  }

  benchmark::DoNotOptimize(counters);
  if (state.thread_index() == 0) {
    // Nothing
  }
}
}  // namespace evpp

// Boost
namespace boost_test {

struct Event {
  uint64_t size;
  uint64_t time;
};

static void BM_C_DispatchSingleHandlersEvent(benchmark::State& state) {
  uint64_t sum = 0;

  boost::signals2::signal<void(const Event&)> signal;
  auto connect = signal.connect([&](const Event& e) { sum += e.size * e.time; });

  for (auto _ : state) {
    signal({1, 2});
  }

  benchmark::DoNotOptimize(sum);
}

}  // namespace boost_test

namespace sigslot_test {

struct Event {
  uint64_t size;
  uint64_t time;
};

static void BM_C_DispatchSingleHandlersEvent(benchmark::State& state) {
  uint64_t sum = 0;

  sigslot::signal<const Event&> signal;
  signal.connect([&](const Event& e) { sum += e.size * e.time; });

  for (auto _ : state) {
    signal(Event{1, 2});
  }

  benchmark::DoNotOptimize(sum);
}

}  // namespace sigslot_test

BENCHMARK(event::test::BM_C_DispatchSingleHandlersEvent);
BENCHMARK(evpp::BM_C_DispatchSingleHandlersEvent);
BENCHMARK(boost_test::BM_C_DispatchSingleHandlersEvent);
BENCHMARK(sigslot_test::BM_C_DispatchSingleHandlersEvent);

BENCHMARK(event::test::BM_SCMP_DispatchSingleHandlersEvent)->ThreadRange(1, 8);
BENCHMARK(evpp::BM_SCMP_DispatchSingleHandlersEvent)->ThreadRange(1, 8);