#pragma once

#include <asio.hpp>

namespace exe::runtime::detail {

class AsioRuntime {
 public:
  template <typename F>
  static void submit(F&& callable) {
    asio::post(thread_pool_, std::forward<F>(callable));
  }

  static void stop() {
    thread_pool_.join();
  }

 private:
  static asio::thread_pool thread_pool_;
};

}  // namespace exe::runtime::detail