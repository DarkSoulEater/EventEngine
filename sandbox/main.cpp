#include <iostream>
#include <event/event.h>
#include "h1.h"

struct MyEvent : public event::EventBase<MyEvent> {
  int a;
};

struct MyEvent2 : public event::EventBase<MyEvent2> {
  int b;
};

int main() {
  std::cout << "Hello world\n";

  event::handle([] {

  });

  MyEvent event;
  std::cout << event.getTypeId() << "\n";

  std::cout << MyEvent2::getTypeId() << "\n";

  print();
}