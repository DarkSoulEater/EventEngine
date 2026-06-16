#include <exe/runtime/asio/runtime.h>

namespace exe::runtime::detail {

asio::thread_pool AsioRuntime::thread_pool_{1};

}  // namespace exe::runtime::detail