#ifndef TASK_TEST_MANAGER_HH
#define TASK_TEST_MANAGER_HH

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
    _manager->stop().get();
  };

  void setup(size_t poolWorkersCount, size_t managerWorkersCount) {
    std::shared_future<void> future = _lock.get_future();

    _threadpool = std::make_shared<Detail::Threadpool>(poolWorkersCount);
    _manager = std::make_shared<Manager>(_threadpool, managerWorkersCount);

    auto task = [future = std::move(future)] {
      EXPECT_EQ(std::future_status::ready, future.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
    };
    _manager->launch(std::move(task));
  }

  void addTasks(std::vector<std::function<void()>>&& functors) {
    for (auto&& functor : functors) {
      _futures.push_back(_manager->launch(functor));
    }
  }
  void runTasks() {
    _lock.set_value();
    for (auto& future : _futures) {
      EXPECT_EQ(std::future_status::ready, future.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
    }
    _futures.clear();
  }

  std::future<void> makeBlockingTask() {
    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();

    auto task = [promise = std::move(promise)] {
      promise->set_value();
    };
    _manager->launch(std::move(task));
    return future;
  }

 private:
  std::shared_ptr<Detail::Threadpool> _threadpool;
  std::shared_ptr<Manager> _manager;
  std::promise<void> _lock;
  std::vector<std::shared_future<void>> _futures;
};

} /* namespace Task */

#endif
