#include "eventpp.h"

#include <iostream>

#include <eventpp/eventdispatcher.h>

using EventppDispatcheer = eventpp::EventDispatcher<int, void(int)>;

void EventppTest() {
  std::cout << "Start Eventpp test... \n";

  EventppDispatcheer dispatcher;
  // dispatcher.appendListener( )
}

void evpp::PerfTest() {
  Dispatcher dispatcher;

  dispatcher.appendListener(0, [](const Event& e) {

  });
}