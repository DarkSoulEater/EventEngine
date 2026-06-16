#pragma once

#include "asio/runtime.h"

namespace exe::runtime::detail {

template <typename F>
void Submit(F&& callable) {
  AsioRuntime::submit(std::forward<F>(callable));
}

static inline void Stop() {
  AsioRuntime::stop();
}

static inline void Wait() {
  AsioRuntime::wait();
}

}  // namespace exe::runtime::detail