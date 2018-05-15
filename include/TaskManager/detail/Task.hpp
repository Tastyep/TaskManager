#ifndef TASK_MANAGER_DETAIL_TASK_MANAGER_HPP
#define TASK_MANAGER_DETAIL_TASK_MANAGER_HPP

#include <chrono>
#include <functional>

namespace Task {
namespace Detail {

using Task = std::function<void()>;
using Clock = std::chrono::steady_clock;
using Timepoint = Clock::time_point;

class TimedTask {
 public:
  TimedTask(Task functor, Timepoint timepoint)
    : _functor(functor)
    , _timepoint(timepoint) {}

  const Timepoint& timepoint() const {
    return _timepoint;
  }

  bool operator>(const TimedTask& other) const {
    return _timepoint > other._timepoint;
  }
  void operator()() {
    _functor();
  }

 private:
  Task _functor;
  Timepoint _timepoint;
};

} /* namespace Detail */
} /* namespace Task */

#endif
