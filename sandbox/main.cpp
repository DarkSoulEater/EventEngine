#include <asio.hpp>
#include <chrono>
#include <iostream>
#include <thread>

#include <event/event.h>
#include <exe/run.h>

#include "h1.h"

struct MyEvent : public event::EventBase<MyEvent> {
  int a;
};

struct MyEvent2 : public event::EventBase<MyEvent2> {
  int b;
};

void Asio() {
  std::cout << "[Main] Отправка задач в asio::thread_pool...\n";

  for (int i = 1; i <= 8; ++i) {
    exe::Run([i]() {
      std::cout << "[Поток " << std::this_thread::get_id() << "] Выполнение задачи " << i << "\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(300));
    });
  }

  std::cout << "[Main] Ожидание завершения всех задач в пуле...\n";

  // join() блокирует текущий поток до завершения всех задач в пуле
  // и корректно останавливает внутренние потоки.
  // pool.join();
  exe::Stop();

  std::cout << "[Main] Готово.\n";
}

struct TimeEvent : public event::EventBase<TimeEvent> {
  using Time = decltype(std::chrono::high_resolution_clock::now());
  Time     start;
  uint64_t id;

  TimeEvent(Time start, uint64_t id) : start(start), id(id) {}
};

struct alignas(std::hardware_destructive_interference_size) Counter {
  uint64_t count;
};

struct Counters {
  Counter counters[100];
};

static Counters counters;

void TimeBench() {
  event::asyncSubscribe<TimeEvent>([](const TimeEvent& e) {
    auto end = std::chrono::high_resolution_clock::now();
    auto dt  = std::chrono::duration<uint64_t, std::nano>(end - e.start).count();
    counters.counters[e.id].count = dt;
  });

  std::this_thread::sleep_for(std::chrono::seconds(10));

  for (size_t i = 0; i < 1; ++i) {
    event::dispatch<TimeEvent>(TimeEvent{std::chrono::high_resolution_clock::now(), i});
  }

  exe::Wait();
  for (size_t k = 0; k < 100; ++k) {
    std::cout << counters.counters[k].count << "\n";
  }
}

int main() {
  TimeBench();
  return 0;
  Asio();

  std::cout << "Hello world\n";

  // event::handle([] {
  //
  // });

  MyEvent event;
  std::cout << event.getTypeId() << "\n";

  std::cout << MyEvent2::getTypeId() << "\n";

  print();
}