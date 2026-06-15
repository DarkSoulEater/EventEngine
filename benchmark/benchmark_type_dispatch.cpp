#include <cstdint>
#include <memory>
#include <random>
#include <typeinfo>
#include <vector>

#include <benchmark/benchmark.h>

// ========================================================================
// 1. Подход RTTI (typeid)
// ========================================================================
struct RTTI_Base {
  virtual ~RTTI_Base() = default;
};
struct RTTI_Event : RTTI_Base {
  int a = 10;
};
struct RTTI_Event1 : RTTI_Base {
  int a = 13;
};
struct RTTI_Event2 : RTTI_Base {
  int a = 12;
};
struct RTTI_Event3 : RTTI_Base {
  int a = 11;
};

// ========================================================================
// 2. Подход виртуальных функций (virtual getTypeId)
// ========================================================================
struct Virt_Base {
  virtual size_t getTypeId() const = 0;
  virtual ~Virt_Base()             = default;
};
struct Virt_Event : Virt_Base {
  size_t getTypeId() const override { return 1; }
};
struct Virt_Event1 : Virt_Base {
  size_t getTypeId() const override { return 2; }
};
struct Virt_Event2 : Virt_Base {
  size_t getTypeId() const override { return 3; }
};
struct Virt_Event3 : Virt_Base {
  size_t getTypeId() const override { return 4; }
};

// ========================================================================
// 3. Подход CRTP (статическая идентификация)
// ========================================================================
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
  explicit EventBase() {}

  ~EventBase() = default;

  inline static size_t getTypeId() {
    static const size_t type_id = nextId();
    return type_id;
  }
};

struct CRTP_Event : EventBase<CRTP_Event> {
  int a = 1;
};

struct CRTP_Event1 : EventBase<CRTP_Event1> {
  int a = 1;
};

struct CRTP_Event2 : EventBase<CRTP_Event2> {
  int a = 1;
};

struct CRTP_Event3 : EventBase<CRTP_Event3> {
  int a = 1;
};

// ========================================================================
// Бенчмарки
// ========================================================================

// 1. Измерение задержки typeid().hash_code()
static void BM_GetId_RTTI(benchmark::State& state) {
  const size_t            n = state.range(0);
  std::vector<RTTI_Base*> objects;

  objects.reserve(n);
  for (size_t k = 0; k < n; ++k) {
    objects.push_back(new RTTI_Event());
  }

  // Целевой type_info для сравнения (эмулирует поиск в реестре)
  const std::type_info& target = typeid(RTTI_Event);

  for (auto _ : state) {
    for (size_t i = 0; i < n; ++i) {
      // typeid требует разыменования vptr, обращения к метаданным и хеширования
      benchmark::DoNotOptimize(typeid(*objects[i]).hash_code());
    }
  }

  for (auto* p : objects)
    delete p;
  state.SetItemsProcessed(state.iterations() * n);
}

// 2. Измерение задержки виртуального вызова getTypeId()
static void BM_GetId_Virtual(benchmark::State& state) {
  const size_t            n = state.range(0);
  std::vector<Virt_Base*> objects;

  objects.reserve(n);
  for (size_t k = 0; k < n; ++k) {
    objects.push_back(new Virt_Event());
  }

  for (auto _ : state) {
    for (size_t i = 0; i < n; ++i) {
      // Косвенный вызов через vtable + возврат константы
      benchmark::DoNotOptimize(objects[i]->getTypeId());
    }
  }

  for (auto* p : objects)
    delete p;
  state.SetItemsProcessed(state.iterations() * n);
}

// 3. Измерение задержки CRTP-идентификации
static void BM_GetId_CRTP(benchmark::State& state) {
  const size_t n = state.range(0);

  // CRTP не требует динамического выделения, используем простой цикл
  volatile size_t sink = 0;
  for (auto _ : state) {
    for (size_t i = 0; i < n; ++i) {
      // Статический вызов: компилятор подставляет константу или загружает из .rodata
      benchmark::DoNotOptimize(sink = CRTP_Event::getTypeId());
    }
  }
  state.SetItemsProcessed(state.iterations() * n);
}

// Регистрация бенчмарков с различными объёмами данных
// BENCHMARK(BM_GetId_RTTI)->RangeMultiplier(4)->Range(256, 65536)->Unit(benchmark::kNanosecond);
// BENCHMARK(BM_GetId_Virtual)->RangeMultiplier(4)->Range(256, 65536)->Unit(benchmark::kNanosecond);
// BENCHMARK(BM_GetId_CRTP)->RangeMultiplier(4)->Range(256, 65536)->Unit(benchmark::kNanosecond);

BENCHMARK_MAIN();