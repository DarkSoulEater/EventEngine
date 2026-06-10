#pragma once

#include "runtime/runtime.h"

namespace exe {

template <typename F>
void Run(F&& callable) {
  runtime::detail::Submit(std::forward<F>(callable));
}

static inline void Stop() {
  runtime::detail::Stop();
}
}  // namespace exe