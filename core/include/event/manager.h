//#ifndef CORE_EVENT_MANAGER_H
//#define CORE_EVENT_MANAGER_H
//
//#include <iostream>
//#include <memory>
//#include <unordered_map>
//
//#include <cds/container/rwqueue.h>
//#include <concurrencpp/concurrencpp.h>
//#include <concurrencpp/runtime/runtime.h>
//
//#include "pscore/engine/concurrency/wait_group.h"
//#include "pscore/event/event.h"
//#include "pscore/event/handler.h"
//#include "pscore/event/pool_allocator.h"
//#include "pscore/managers/manager_base.h"
//#include "pscore/managers/manager_config.h"
//#include "pscore/managers/manager_context.h"
//#include "pscore/utils/debug.h"
//
//namespace Core {
//
//template <typename T>
//concept HasTypeId = requires(T t) {
//  { T::getTypeId() } -> std::same_as<size_t>;
//};
//
//class EventManager final : public ManagerI {
// public:
//  static constexpr std::string_view mName = "event-core";
//
//  EventManager(const ManagerConfig&, const ManagerContext&);
//
//  template <HasTypeId EventType, typename... Args>
//  void dispatch(Args&&... args) {
//    void* memory = allocator_.allocate<EventType>();
//    PS_ASSERT(memory);
//
//    auto* event = new (memory) EventType(std::forward<Args>(args)...);
//    if (!queue_.enqueue(Node(EventType::getTypeId(), event, allocator_))) {
//      PS_ASSERT(0);
//    }
//
//    event_queue_wg_.add(1);
//  }
//
//  template <HasTypeId EventType>
//  void subscribe(std::function<void(const EventType&)> callback) {
//    subscribers_[EventType::getTypeId()].emplace_back(std::move(callback));
//  }
//
//  void wait() {
//    event_queue_wg_.wait();
//    handlers_wg_.wait();
//  }
//
// private:
//  void initialize() override;
//
//  void finalize() override;
//
//  void process();
//
// private:
//  typedef EventBase<void> Event;
//  struct Node {
//    size_t                 type_id;
//    std::shared_ptr<Event> event;
//
//    Node() : type_id(0), event(nullptr) {}
//
//    template <typename EventType>
//    Node(size_t type_id, EventType* event, MultiSizePoolAllocator& allocator_)
//        : type_id(type_id), event((Event*)event, [&allocator_](Event* ptr) {
//          ((EventType*)ptr)->~EventType();
//          allocator_.free<EventType>(ptr);
//        }) {}
//  };
//
//  cds::container::RWQueue<Node>                    queue_;
//  concurrencpp::runtime                            runtime_;
//  concurrencpp::timer                              process_timer_;
//  std::unordered_map<size_t, std::vector<Handler>> subscribers_;
//  concurrency::WaitGroup                           handlers_wg_;
//  concurrency::WaitGroup                           event_queue_wg_;
//  MultiSizePoolAllocator                           allocator_;
//
//  constexpr static std::chrono::milliseconds process_due_time_{10};
//  constexpr static std::chrono::milliseconds process_frequency_{100};
//};
//
//}  // namespace Core
//
//#endif  // CORE_EVENT_MANAGER_H