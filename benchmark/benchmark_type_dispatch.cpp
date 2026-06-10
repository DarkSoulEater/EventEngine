#include <benchmark/benchmark.h>
#include <vector>
#include <typeinfo>
#include <cstdint>
#include <memory>

// ============================================================================
// 1. Подход на базе RTTI (typeid)
// ============================================================================
struct RTTIBase {
  virtual ~RTTIBase() = default;
  int64_t payload = 0;
};

struct RTTIEvent : RTTIBase {
  static constexpr const char* name() { return "RTTIEvent"; }
};

// ============================================================================
// 2. Подход на базе виртуальных функций (динамическая диспетчеризация)
// ============================================================================
struct VirtBase {
  virtual void handle() = 0;
  virtual ~VirtBase() = default;
  int64_t payload = 0;
};

struct VirtEvent : VirtBase {
  void handle() override {
    benchmark::DoNotOptimize(payload);
  }
};

// ============================================================================
// 3. CRTP + статическая линейная идентификация (предлагаемый подход)
// ============================================================================
struct IdGenerator {
  static std::size_t next() {
    static std::size_t counter = 0;
    return counter++;
  }
};

template <typename Derived>
struct CRTPBase {
  static std::size_t typeId() noexcept {
    static const std::size_t id = IdGenerator::next();
    return id;
  }
};

struct CRTPEvent : CRTPBase<CRTPEvent> {
  int64_t payload = 0;
};

// Имитация обработчика в таблице маршрутизации
static void crtp_handler(const CRTPEvent* evt) {
  benchmark::DoNotOptimize(evt->payload);
}

// ============================================================================
// Бенчмарки
// ============================================================================
constexpr int kEventCount = 100'000;

// 1. Измерение накладных расходов typeid
static void BM_Identify_typeid(benchmark::State& state) {
  std::vector<RTTIBase*> events(state.range(0));
  for (auto& e : events) e = new RTTIEvent();
    
  const std::type_info& target = typeid(RTTIEvent);
  volatile std::size_t matched = 0;

  for (auto _ : state) {
    for (const auto* e : events) {
      if (typeid(*e) == target) {
        benchmark::DoNotOptimize(++matched);
      }
    }
  }
    
  for (auto* e : events) delete e;
}

// 2. Измерение накладных расходов виртуального вызова
static void BM_Dispatch_virtual(benchmark::State& state) {
  std::vector<VirtBase*> events(state.range(0));
  for (auto& e : events) e = new VirtEvent();

  for (auto _ : state) {
    for (auto* e : events) {
      e->handle();
    }
  }

  for (auto* e : events) delete e;
}

// 3. Измерение накладных расходов CRTP-маршрутизации
static void BM_Dispatch_crtp(benchmark::State& state) {
  std::vector<CRTPEvent*> events(state.range(0));
  for (auto& e : events) e = new CRTPEvent();
    
  // Имитация получения указателя из таблицы обработчиков по type_id
  using HandlerPtr = void(*)(const CRTPEvent*);
  HandlerPtr handler = crtp_handler;

  for (auto _ : state) {
    for (const auto* e : events) {
      handler(e);
    }
  }

  for (auto* e : events) delete e;
}

// 4. Измерение скорости получения идентификатора типа
static void BM_GetTypeId_runtime(benchmark::State& state) {
  // Предварительная инициализация статической переменной
  [[maybe_unused]] std::size_t dummy = CRTPEvent::typeId();
    
  volatile std::size_t sink = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(sink = CRTPEvent::typeId());
  }
}

// Регистрация бенчмарков
BENCHMARK(BM_Identify_typeid)->Arg(kEventCount)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_Dispatch_virtual)->Arg(kEventCount)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_Dispatch_crtp)->Arg(kEventCount)->Unit(benchmark::kMicrosecond);
BENCHMARK(BM_GetTypeId_runtime)->Iterations(1000000)->Unit(benchmark::kNanosecond);

BENCHMARK_MAIN();