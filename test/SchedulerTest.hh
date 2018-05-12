#ifndef TASK_TEST_SCHEDULER_TEST_HH
#define TASK_TEST_SCHEDULER_TEST_HH

#include "gtest/gtest.h"

#include <functional>
#include <utility>
#include <vector>

#include "test/Async.hh"

#include "Scheduler.hh"
#include "detail/Threadpool.hh"

using testing::Test;

namespace Task {

class SchedulerTest : public Test {
 public:
  SchedulerTest() = default;
  ~SchedulerTest() {
    if (_scheduler) {
      auto done = _scheduler->stop();
      EXPECT_EQ(std::future_status::ready, done.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
    }
  };

  void setup(size_t poolWorkersCount, size_t schedulerWorkersCount) {
    _threadpool = std::make_shared<Detail::Threadpool>(poolWorkersCount);
    _scheduler = this->setupScheduler(schedulerWorkersCount);
  }

  std::shared_ptr<Scheduler> setupScheduler(size_t schedulerWorkersCount) {
    if (!_lock.first) {
      _lock.first = std::make_unique<std::promise<void>>();
      _lock.second = _lock.first->get_future();
    }

    auto scheduler = std::make_shared<Scheduler>(_threadpool, schedulerWorkersCount);
    for (size_t i = 0; i < schedulerWorkersCount; ++i) {
      auto task = [this] {
        EXPECT_EQ(std::future_status::ready, _lock.second.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
      };
      scheduler->scheduleIn("setup_" + std::to_string(i), -std::chrono::hours(1), std::move(task));
    }

    return scheduler;
  }

  void addTasks(std::vector<std::pair<std::function<void()>, std::chrono::microseconds>>&& functors) {
    const auto now = Detail::Clock::now();
    for (size_t i = 0; i < functors.size(); ++i) {
      _futures.push_back(_scheduler->scheduleAt(std::to_string(i), now + functors[i].second, functors[i].first));
    }
  }
  void runTasks() {
    if (_lock.first) {
      _lock.first->set_value();
    }
    for (auto& future : _futures) {
      EXPECT_EQ(std::future_status::ready, future.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
    }
    _futures.clear();
  }

 protected:
  std::shared_ptr<Detail::Threadpool> _threadpool;
  std::shared_ptr<Scheduler> _scheduler;
  std::pair<std::unique_ptr<std::promise<void>>, std::shared_future<void>> _lock;
  std::vector<std::shared_future<void>> _futures;
};

} /* namespace Task */

#endif
