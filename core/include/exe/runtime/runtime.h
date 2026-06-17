#pragma once

#include "asio/runtime.h"

namespace exe::runtime::detail {

template <typename F>
void Submit(F&& callable) {
  AsioRuntime::submit(std::forward<F>(callable));
  // MultiThread::submit(std::forward<F>(callable));
}

static inline void Start() {
  // MultiThread::start();
}

static inline void Stop() {
  AsioRuntime::stop();
}

static inline void Wait() {
  AsioRuntime::wait();
}

}  // namespace exe::runtime::detail