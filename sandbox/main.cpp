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

int main() {
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