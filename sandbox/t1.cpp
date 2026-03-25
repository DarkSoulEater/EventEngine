#include "h1.h"
#include <event/event.h>
#include <iostream>

struct MyEvent3 : public event::EventBase<MyEvent3> {
  char c;
};

void print() {
  std::cout << MyEvent3::getTypeId() << "\n";
}