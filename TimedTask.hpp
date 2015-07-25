#ifndef TIMEDTASK_HPP_
#define TIMEDTASK_HPP_

#include <chrono>

#include "Task.hpp"

class TimedTask : public Task {
  TimedTask(const std::function<void ()>& func,
            const std::chrono::milliseconds& ms) :
  Task(func), interval(ms) {}
  ~TimedTask() = default;

  const std::chrono::milliseconds& getInterval() const;

private:
  std::chrono::milliseconds interval;
}

#endif /* end of include guard: TIMEDTASK_HPP_ */
