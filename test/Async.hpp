#ifndef TASK_MANAGER_TEST_ASYNC_HPP
#define TASK_MANAGER_TEST_ASYNC_HPP

#include <chrono>

namespace Async {

constexpr std::chrono::milliseconds kTestTimeout = std::chrono::milliseconds(30'000);

} /* namespace Async */

#endif
