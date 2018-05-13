#ifndef TASK_TEST_ASYNC_HH
#define TASK_TEST_ASYNC_HH

#include <chrono>

namespace Async {

constexpr std::chrono::milliseconds kTestTimeout = std::chrono::milliseconds(30'000);

} /* namespace Async */

#endif
