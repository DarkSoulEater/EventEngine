// test_performance.cpp
// #include <gtest/gtest.h>
#include <chrono>
#include <complex>
#include <vector>

#include <benchmark/benchmark.h>
#include <event/event.h>
#include <event/handler.h>
#include <exe/thread/wait_group.h>

namespace event::test {

struct StubEvent : public EventBase<StubEvent> {
  int a = 0;
};

struct PerfEvent : public EventBase<PerfEvent> {
  uint64_t id;
  double   metric;

  PerfEvent(uint64_t i, double m) : id(i), metric(m) {}
};

struct PromiseEvent : public EventBase<PromiseEvent> {
  mutable std::promise<void> p;

  explicit PromiseEvent(std::promise<void> p) : p(std::move(p)) {}
};

struct WGEvent : public EventBase<WGEvent> {
  exe::thread::WaitGroup& wg;

  explicit WGEvent(exe::thread::WaitGroup& wg) : wg(wg) {}
};

// Бенчмарк 4.1: Скорость диспетчеризации без обработчиков
static void BM_DispatchNoHandlers(benchmark::State& state) {
  for (auto _ : state) {
    dispatch<PerfEvent>(state.range(0), 3.14159);
  }
}

BENCHMARK(BM_DispatchNoHandlers)->Arg(1)->Arg(100)->Arg(10000);

// Бенчмарк 4.2: Диспетчеризация с одним обработчиком
static void BM_DispatchSingleHandler(benchmark::State& state) {
  uint64_t sink = 0;

  syncSubscribe<PerfEvent>([&sink](const PerfEvent& evt) { sink += evt.id; });

  const auto count = state.range(0);

  for (auto _ : state) {
    for (size_t k = 0; k < count; ++k) {
      dispatch<PerfEvent>(count, 2.71828);
    }
  }

  // if (sink != state.iterations() * PerfEvent::getTypeId()) {
  //   std::abort();
  // }

  benchmark::DoNotOptimize(sink);
  detail::clearHandlers();
}
BENCHMARK(BM_DispatchSingleHandler)->Arg(1)->Arg(100)->Arg(10000);

// Бенчмарк 4.3: Множественные обработчики
static void BM_DispatchMultipleHandlers(benchmark::State& state) {
  const int             handler_count = state.range(1);
  std::vector<uint64_t> sinks(handler_count, 0);

  const auto count = state.range(0);

  for (int i = 0; i < handler_count; ++i) {
    syncSubscribe<PerfEvent>([&sinks, i](const PerfEvent& evt) { sinks[i] += evt.id * (i + 1); });
  }

  for (auto _ : state) {
    for (size_t k = 0; k < count; ++k) {
      dispatch<PerfEvent>(state.range(0), 1.41421);
    }
  }

  for (auto& s : sinks) {
    benchmark::DoNotOptimize(s);
  }

  detail::clearHandlers();
}
BENCHMARK(BM_DispatchMultipleHandlers)
    ->Args({1000, 1})
    ->Args({1000, 4})
    ->Args({1000, 8})
    ->Args({1000, 16});

struct alignas(std::hardware_destructive_interference_size) Counter {
  uint64_t count{0};
};

struct Counters {
  Counter counter[8];
};

static void BM_SyncDispatchSingleHandlerMT(benchmark::State& state) {
  static Counters counters;

  if (state.thread_index() == 0) {
    syncSubscribe<PerfEvent>([](const PerfEvent& evt) {
      counters.counter[evt.id].count += static_cast<size_t>(evt.metric);
    });
  }

  const auto thread_id = state.thread_index();

  for (auto _ : state) {
    for (size_t k = 0; k < 10000; ++k) {
      dispatch<PerfEvent>(thread_id, 2.0);
    }
  }

  benchmark::DoNotOptimize(counters);

  if (state.thread_index() == 0) {
    detail::clearHandlers();
  }
}
BENCHMARK(BM_SyncDispatchSingleHandlerMT)
    ->ThreadRange(1, 8)
    ->MeasureProcessCPUTime()
    ->UseRealTime();

constexpr size_t kIterCount = 10000000;

static void BM_SyncAndAsyncCompareSync(benchmark::State& state) {
  static Counters counters;

  syncSubscribe<PerfEvent>([](const PerfEvent& evt) {
    auto val = evt.id * (evt.metric + 1);
    benchmark::DoNotOptimize(val);
    //    counters.counter[evt.id].count += static_cast<size_t>(evt.metric);
  });

  syncSubscribe<WGEvent>([](const WGEvent& e) { e.wg.done(); });

  for (auto _ : state) {
    exe::thread::WaitGroup wg;

    wg.add(1);
    for (size_t k = 0; k < kIterCount; ++k) {
      dispatch<PerfEvent>(1, 2);
    }
    dispatch<WGEvent>(wg);

    wg.wait();
  }

  benchmark::DoNotOptimize(counters);

  detail::clearHandlers();
}

static void BM_SyncAndAsyncCompareAsync(benchmark::State& state) {
  static Counters counters;

  asyncSubscribe<PerfEvent>([](const PerfEvent& evt) {
    auto val = evt.id * (evt.metric + 1);
    benchmark::DoNotOptimize(val);
    //    counters.counter[evt.id].count += static_cast<size_t>(evt.metric);
  });

  asyncSubscribe<WGEvent>([](const WGEvent& e) { e.wg.done(); });

  for (auto _ : state) {
    exe::thread::WaitGroup wg;

    wg.add(1);
    for (size_t k = 0; k < kIterCount; ++k) {
      dispatch<PerfEvent>(1, 2);
    }
    dispatch<WGEvent>(wg);

    wg.wait();
  }

  benchmark::DoNotOptimize(counters);

  detail::clearHandlers();
}

BENCHMARK(BM_SyncAndAsyncCompareSync);
BENCHMARK(BM_SyncAndAsyncCompareAsync);

// //
// //// Бенчмарк 4.4: Аллокация через пул vs new/delete
// //static void BM_PoolAllocation(benchmark::State& state) {
// //  for (auto _ : state) {
// //    auto* evt = MemoryPool<PerfEvent>::allocate();
// //    new (evt) PerfEvent(42, 1.0);
// //    evt->~PerfEvent();
// //    MemoryPool<PerfEvent>::deallocate(evt);
// //  }
// //}
// //BENCHMARK(BM_PoolAllocation);
//
// static void BM_StandardAllocation(benchmark::State& state) {
//   for (auto _ : state) {
//     auto* evt = new PerfEvent(42, 1.0);
//     delete evt;
//   }
// }
// BENCHMARK(BM_StandardAllocation);
//
// // Интеграционный тест производительности
// //TEST(PerformanceTest, ThroughputMeasurement) {
// //  EventRegistry::instance().clear();
// //  AsyncQueue::instance().start_workers(8);
// //
// //  std::atomic<uint64_t> processed{0};
// //
// //  subscribe<PerfEvent>([&](const PerfEvent&) {
// //    ++processed;
// //  });
// //
// //  const uint64_t target_events = 1'000'000;
// //  auto start = std::chrono::high_resolution_clock::now();
// //
// //  for (uint64_t i = 0; i < target_events; ++i) {
// //    dispatch<PerfEvent>(i, static_cast<double>(i) * 0.001);
// //  }
// //
// //  // Ожидаем завершения обработки
// //  while (processed < target_events) {
// //    std::this_thread::sleep_for(std::chrono::milliseconds(10));
// //  }
// //
// //  auto end = std::chrono::high_resolution_clock::now();
// //  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
// //
// //  double events_per_second = (target_events * 1'000'000.0) / duration.count();
// //
// //  EXPECT_GT(events_per_second, 500'000); // Минимальный порог: 500K events/sec
// //
// //  AsyncQueue::instance().stop_workers();
// //
// //  std::cerr << "Throughput: " << events_per_second / 1'000'000
// //            << "M events/sec" << std::endl;
// //}

}  // namespace event::test