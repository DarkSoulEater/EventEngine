#ifndef CORE_CONCURRENCY_WAIT_GROUP
#define CORE_CONCURRENCY_WAIT_GROUP

#include <condition_variable>
#include <cstddef>
#include <mutex>

namespace exe::thread {
class WaitGroup {
 public:
  void add(size_t count) {
    std::lock_guard lock(mutex_);
    work_counter_ += count;
  }

  void done() {
    std::lock_guard lock(mutex_);
    work_counter_ -= 1;

    if (waiters_ && work_counter_ == 0) {
      waiters_ = false;
      is_work_done_.notify_all();
    }
  }

  void wait() {
    std::unique_lock lock(mutex_);

    while (work_counter_ != 0) {
      waiters_ = true;
      is_work_done_.wait(lock);
    }
  }

 private:
  uint64_t                work_counter_{0};
  std::mutex              mutex_;
  std::condition_variable is_work_done_;
  bool                    waiters_{false};
};

};  // namespace exe::thread

#endif  // CORE_CONCURRENCY_WAIT_GROUP