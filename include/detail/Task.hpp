#ifndef TASK_DETAIL_TASK_HPP
#define TASK_DETAIL_TASK_HPP

#include <chrono>
#include <functional>

namespace Task {
namespace Detail {

using Task = std::function<void()>;
using Clock = std::chrono::steady_clock;
using Timepoint = Clock::time_point;

struct TimedTask {
  Task functor;
  Timepoint timepoint;

  bool operator>(const TimedTask& other) const {
    return timepoint < other.timepoint;
  }
  void operator()() {
    functor();
  }
};

} /* namespace Detail */
} /* namespace Task */

#endif
