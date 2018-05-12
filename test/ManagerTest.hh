#ifndef TASK_TEST_MANAGER_TEST_HH
#define TASK_TEST_MANAGER_TEST_HH

#include "gtest/gtest.h"

#include <functional>
#include <vector>

#include "test/Async.hh"

#include "Manager.hh"
#include "detail/Threadpool.hh"

using testing::Test;

namespace Task {

class ManagerTest : public Test {
 public:
  ManagerTest() = default;
  ~ManagerTest() {
    if (_manager) {
      auto done = _manager->stop();
      EXPECT_EQ(std::future_status::ready, done.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
    }
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
    _manager = std::make_shared<Manager>(_threadpool, managerWorkersCount);

    for (size_t i = 0; i < futures.size(); ++i) {
      auto task = [future = std::move(futures[i])] {
        EXPECT_EQ(std::future_status::ready, future.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
      };
      _manager->launch(std::move(task));
    }
  }

  void addTasks(std::vector<std::function<void()>>&& functors) {
    for (auto&& functor : functors) {
      _futures.push_back(_manager->launch(functor));
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

 protected:
  std::shared_ptr<Detail::Threadpool> _threadpool;
  std::shared_ptr<Manager> _manager;
  std::vector<std::promise<void>> _locks;
  std::vector<std::shared_future<void>> _futures;
};

} /* namespace Task */

#endif
