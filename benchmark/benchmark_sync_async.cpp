#include <chrono>
#include <complex>
#include <vector>

#include <benchmark/benchmark.h>
#include <event/event.h>
#include <event/handler.h>
#include <exe/thread/wait_group.h>

// ========================================================================
// Вспомогательные функции
// ========================================================================
inline uint64_t now_ns() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::high_resolution_clock::now().time_since_epoch()
  )
      .count();
}

// ========================================================================
// Тип события
// ========================================================================
struct BenchmarkEvent : public event::EventBase<BenchmarkEvent> {
  uint32_t payload;
  BenchmarkEvent(uint32_t p) : EventBase(), payload(p) {}
};

struct WaitEvent : public event::EventBase<WaitEvent> {
  exe::thread::WaitGroup& wg;

  explicit WaitEvent(exe::thread::WaitGroup& wg) : wg(wg) {}
};

// ========================================================================
// Эмуляция типов обработчиков
// ========================================================================

// 1. Легковесный CPU-bound обработчик (минимальная обработка)
inline void lightweight_cpu_work() {
  benchmark::DoNotOptimize(42 * 2);
}

// 2. Тяжёлый CPU-bound обработчик (~100 нс работы)
inline void heavy_cpu_work() {
  volatile uint64_t result = 0;
  for (int i = 0; i < 100; ++i) {
    result += i * i;
  }
  benchmark::DoNotOptimize(result);
}

// 3. IO-bound обработчик (симуляция ~1 мкс ожидания)
inline void io_bound_work() {
  // Эмуляция блокирующей операции (например, запись в лог или сетевой вызов)
  //  std::this_thread::sleep_for(std::chrono::microseconds(1));
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

namespace test::sync {
static void BM_AS_LightweightCPU_Sync(benchmark::State& state) {
  std::atomic<uint64_t> complete_time{0};

  event::syncSubscribe<BenchmarkEvent>([&complete_time](const BenchmarkEvent& evt) {
    lightweight_cpu_work();

    //    state.PauseTiming();
    // complete_time.store(now_ns(), std::memory_order_relaxed);
    //    state.ResumeTiming();
  });

  for (auto _ : state) {
    //    state.PauseTiming();
    auto start = 0;  // now_ns();
                     //    state.ResumeTiming();

    event::dispatch<BenchmarkEvent>(start);
    auto publish_latency = start - start;  // В синхронном режиме измеряем полный цикл
    auto full_latency    = complete_time.load(std::memory_order_relaxed) - start;

    benchmark::DoNotOptimize(publish_latency);
    benchmark::DoNotOptimize(full_latency);
  }

  event::detail::clearHandlers();
}
}  // namespace test::sync

namespace test::async {
static void BM_AS_LightweightCPU_Async(benchmark::State& state) {
  std::atomic<uint64_t> counter{0};

  event::asyncSubscribe<BenchmarkEvent>([&](const BenchmarkEvent& evt) {
    lightweight_cpu_work();
    benchmark::DoNotOptimize(evt.payload);
    counter.fetch_add(1, std::memory_order_relaxed);
  });

  event::asyncSubscribe<WaitEvent>([](const WaitEvent& e) { e.wg.done(); });

  std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Прогрев

  exe::thread::WaitGroup wg;
  for (auto _ : state) {
    counter.store(0, std::memory_order_relaxed);
    wg.add(1);

    for (size_t k = 0; k < 100000; ++k) {
      event::dispatch<BenchmarkEvent>(42);
    }

    event::dispatch<WaitEvent>(wg);

    wg.wait();
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  event::detail::clearHandlers();
}

static void BM_AS_PublishLatency_Async_Lightweight(benchmark::State& state) {
  event::asyncSubscribe<BenchmarkEvent>([](const BenchmarkEvent& evt) {
    lightweight_cpu_work();
    benchmark::DoNotOptimize(evt.payload);
  });

  // Прогрев пула ASIO
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  for (auto _ : state) {
    // Измеряем ТОЛЬКО время до возврата из dispatch
    event::dispatch<BenchmarkEvent>(42);
    // НЕ ждём завершения!
  }

  // Даём время на выполнение всех отложенных задач
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  event::detail::clearHandlers();
}
}  // namespace test::async

namespace test::other {

static void BM_AS_NowNs(benchmark::State& state) {
  for (auto _ : state) {
    auto start = now_ns();
    benchmark::DoNotOptimize(start);
  }
}

static void BM_AS_Delta(benchmark::State& state) {
  for (auto _ : state) {
    auto start = now_ns();
    auto end   = now_ns();
    benchmark::DoNotOptimize(start);
    benchmark::DoNotOptimize(end);
  }
}

static void BM_AS_Sleep(benchmark::State& state) {
  for (auto _ : state) {
    io_bound_work();
  }
}

}  // namespace test::other

// BENCHMARK(test::sync::BM_AS_LightweightCPU_Sync)->Unit(benchmark::kNanosecond);
//BENCHMARK(test::async::BM_AS_LightweightCPU_Async);  //->Unit(benchmark::kNanosecond);
BENCHMARK(test::async::BM_AS_PublishLatency_Async_Lightweight);  //->Unit(benchmark::kNanosecond);

// BENCHMARK(test::other::BM_AS_NowNs);
// BENCHMARK(test::other::BM_AS_Delta);
// BENCHMARK(test::other::BM_AS_Sleep);
