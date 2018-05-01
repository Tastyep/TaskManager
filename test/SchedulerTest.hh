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
    _scheduler->stop().get();
  };

  void setup(size_t poolWorkersCount, size_t managerWorkersCount) {
    for (size_t i = 0; i < poolWorkersCount; ++i) {
      _locks.emplace_back();
    }
    std::vector<std::shared_future<void>> futures(_locks.size());

    for (size_t i = 0; i < _locks.size(); ++i) {
      futures[i] = _locks[i].get_future();
    }

    _threadpool = std::make_shared<Detail::Threadpool>(poolWorkersCount);
    _scheduler = std::make_shared<Scheduler>(_threadpool, managerWorkersCount);

    for (auto&& future : futures) {
      auto task = [future = std::move(future)] {
        EXPECT_EQ(std::future_status::ready, future.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
      };
      _threadpool->schedule(Detail::TimedTask{ std::move(task), Detail::Clock::now() - std::chrono::hours(1) });
    }
  }

  void addTasks(std::vector<std::pair<std::function<void()>, std::chrono::microseconds>>&& functors) {
    const auto now = Detail::Clock::now();
    for (size_t i = 0; i < functors.size(); ++i) {
      _futures.push_back(_scheduler->launchAt(std::to_string(i), now + functors[i].second, functors[i].first));
    }
  }
  void runTasks() {
    for (auto& lock : _locks) {
      lock.set_value();
    }
    for (auto& future : _futures) {
      EXPECT_EQ(std::future_status::ready, future.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
    }
    _futures.clear();
  }

 private:
  std::shared_ptr<Detail::Threadpool> _threadpool;
  std::shared_ptr<Scheduler> _scheduler;
  std::vector<std::promise<void>> _locks;
  std::vector<std::shared_future<void>> _futures;
};

} /* namespace Task */

#endif
