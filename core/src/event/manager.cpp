// #include "pscore/event/manager.h"

namespace Core {

// EventManager::EventManager(const ManagerConfig&, const ManagerContext&) {}
//
// void EventManager::initialize() {
//   using namespace std::chrono_literals;
//
//   process_timer_ = runtime_.timer_queue()->make_timer(
//       process_due_time_, process_frequency_, runtime_.thread_pool_executor(), [this] {
//         this->process();
//       }
//   );
// }
//
// void EventManager::finalize() {
//   wait();
// }
//
// void EventManager::process() {
//   Node node;
//   while (queue_.dequeue(node)) {
//     event_queue_wg_.done();
//
//     auto type_id = node.type_id;
//     auto event   = node.event;
//
//     for (auto& handler : subscribers_[type_id]) {
//       handlers_wg_.add(1);
//
//       runtime_.thread_pool_executor()->submit([&handler, this, event]() {
//         handler.handle(event.get());
//         handlers_wg_.done();
//       });
//     }
//   }
// }

}  // namespace Core