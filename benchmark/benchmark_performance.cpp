// test_performance.cpp
//#include <gtest/gtest.h>
#include <benchmark/benchmark.h>
#include <chrono>
#include <vector>
#include <event/event.h>
#include <event/handler.h>
//#include "event/dispatcher.hpp"
//#include "event/memory_pool.hpp"

namespace event::test {

struct PerfEvent : public EventBase<PerfEvent> {
  uint64_t id;
  double metric;

  PerfEvent(uint64_t i, double m) : id(i), metric(m) {}
};

//// Бенчмарк 4.1: Скорость диспетчеризации без обработчиков
//static void BM_DispatchNoHandlers(benchmark::State& state) {
//  EventRegistry::instance().clear();
//
//  for (auto _ : state) {
//    dispatch<PerfEvent>(state.range(0), 3.14159);
//  }
//
//  EventRegistry::instance().flush();
//}
//BENCHMARK(BM_DispatchNoHandlers)->Arg(1)->Arg(100)->Arg(10000);
//
//// Бенчмарк 4.2: Диспетчеризация с одним обработчиком
//static void BM_DispatchSingleHandler(benchmark::State& state) {
//  EventRegistry::instance().clear();
//
//  volatile uint64_t sink = 0;
//  subscribe<PerfEvent>([&sink](const PerfEvent& evt) {
//    sink += evt.id;
//  });
//
//  for (auto _ : state) {
//    dispatch<PerfEvent>(state.range(0), 2.71828);
//  }
//
//  EventRegistry::instance().flush();
//  benchmark::DoNotOptimize(sink);
//}
//BENCHMARK(BM_DispatchSingleHandler)->Arg(1)->Arg(100)->Arg(10000);
//
//// Бенчмарк 4.3: Множественные обработчики
//static void BM_DispatchMultipleHandlers(benchmark::State& state) {
//  EventRegistry::instance().clear();
//
//  const int handler_count = state.range(1);
//  std::vector<volatile uint64_t> sinks(handler_count, 0);
//
//  for (int i = 0; i < handler_count; ++i) {
//    subscribe<PerfEvent>([&sinks, i](const PerfEvent& evt) {
//      sinks[i] += evt.id * (i + 1);
//    });
//  }
//
//  for (auto _ : state) {
//    dispatch<PerfEvent>(state.range(0), 1.41421);
//  }
//
//  EventRegistry::instance().flush();
//  for (auto& s : sinks) benchmark::DoNotOptimize(s);
//}
//BENCHMARK(BM_DispatchMultipleHandlers)
//    ->Args({100, 1})
//    ->Args({100, 10})
//    ->Args({100, 100});
//
//// Бенчмарк 4.4: Аллокация через пул vs new/delete
//static void BM_PoolAllocation(benchmark::State& state) {
//  for (auto _ : state) {
//    auto* evt = MemoryPool<PerfEvent>::allocate();
//    new (evt) PerfEvent(42, 1.0);
//    evt->~PerfEvent();
//    MemoryPool<PerfEvent>::deallocate(evt);
//  }
//}
//BENCHMARK(BM_PoolAllocation);

static void BM_StandardAllocation(benchmark::State& state) {
  for (auto _ : state) {
    auto* evt = new PerfEvent(42, 1.0);
    delete evt;
  }
}
BENCHMARK(BM_StandardAllocation);

// Интеграционный тест производительности
//TEST(PerformanceTest, ThroughputMeasurement) {
//  EventRegistry::instance().clear();
//  AsyncQueue::instance().start_workers(8);
//
//  std::atomic<uint64_t> processed{0};
//
//  subscribe<PerfEvent>([&](const PerfEvent&) {
//    ++processed;
//  });
//
//  const uint64_t target_events = 1'000'000;
//  auto start = std::chrono::high_resolution_clock::now();
//
//  for (uint64_t i = 0; i < target_events; ++i) {
//    dispatch<PerfEvent>(i, static_cast<double>(i) * 0.001);
//  }
//
//  // Ожидаем завершения обработки
//  while (processed < target_events) {
//    std::this_thread::sleep_for(std::chrono::milliseconds(10));
//  }
//
//  auto end = std::chrono::high_resolution_clock::now();
//  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
//
//  double events_per_second = (target_events * 1'000'000.0) / duration.count();
//
//  EXPECT_GT(events_per_second, 500'000); // Минимальный порог: 500K events/sec
//
//  AsyncQueue::instance().stop_workers();
//
//  std::cerr << "Throughput: " << events_per_second / 1'000'000
//            << "M events/sec" << std::endl;
//}

} // namespace event::test

// Регистрация бенчмарков
BENCHMARK_MAIN();