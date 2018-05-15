#include "test/ManagerTest.hpp"

using namespace std::chrono_literals;

namespace Task {

TEST_F(ManagerTest, launchOne) {
  this->setup(1, 1);
  this->runTasks(); // This makes a task for blocking the manager.
}

TEST_F(ManagerTest, launchMultipleSequentially) {
  size_t i = 0;

  this->setup(1, 1);
  for (size_t j = 0; j < 10; ++j) {
    this->addTasks({
      [&i, j] { i = j; },
    });
  }
  this->runTasks();

  EXPECT_EQ(9, i);
}

TEST_F(ManagerTest, launchMultipleInParallel) {
  this->setup(3, 3);
  for (size_t j = 0; j < 10; ++j) {
    this->addTasks({
      [] {},
    });
  }
  this->runTasks();
}

TEST_F(ManagerTest, launchDependentInParallel) {
  std::promise<void> p1;
  std::promise<void> p2;
  auto f1 = p1.get_future();
  auto f2 = p2.get_future();

  this->setup(2, 2);
  this->addTasks({
    [&p2, &f1] {
      p2.set_value();
      EXPECT_EQ(std::future_status::ready, f1.wait_for(Async::kTestTimeout));
    },
    [&p1, &f2] {
      p1.set_value();
      EXPECT_EQ(std::future_status::ready, f2.wait_for(Async::kTestTimeout));
    },
  });
  this->runTasks();
}

TEST_F(ManagerTest, launchDependentSequentially) {
  std::promise<void> p1;
  std::promise<void> p2;
  auto f1 = p1.get_future();
  auto f2 = p2.get_future();

  this->setup(1, 1);
  this->addTasks({
    [&p2, &f1] {
      p2.set_value();
      EXPECT_EQ(std::future_status::timeout, f1.wait_for(1ns));
    },
    [&p1, &f2] {
      p1.set_value();
      EXPECT_EQ(std::future_status::ready, f2.wait_for(Async::kTestTimeout));
    },
  });
  this->runTasks();
}

TEST_F(ManagerTest, launchNested) {
  this->setup(2, 2);
  this->addTasks({
    [this] {
      std::promise<void> promise;
      auto future = promise.get_future();

      _manager->launch([promise = std::move(promise)]() mutable { promise.set_value(); });

      EXPECT_EQ(std::future_status::ready, future.wait_for(Async::kTestTimeout));
    },
  });

  this->runTasks();
}

TEST_F(ManagerTest, stop) {
  this->setup(2, 2);
  this->addTasks({
    // We make the worker sleep for a short period of time so that when it wakes up, if the stop wasn't waiting for
    // all tasks to complete, it would access to a destroyed manager.
    [] { std::this_thread::sleep_for(5ms); },
  });
  _futures.clear(); // Don't wait for the task to finish before stopping.
  this->runTasks();
}

TEST_F(ManagerTest, launchOnStopped) {
  auto threadpool = std::make_shared<Detail::Threadpool>(2);
  auto manager = std::make_shared<Manager>(threadpool, 1);

  manager->stop().get();
  auto future = manager->launch([] { return true; });

  EXPECT_TRUE(future.valid());
  EXPECT_THROW(future.get(), std::future_error);
}

TEST_F(ManagerTest, multipleManagers) {
  auto threadpool = std::make_shared<Detail::Threadpool>(2);
  auto managerA = std::make_shared<Manager>(threadpool, 1);
  auto managerB = std::make_shared<Manager>(threadpool, 1);
  auto managerC = std::make_shared<Manager>(threadpool, 1);
  std::promise<void> p1;
  std::promise<void> p2;
  std::promise<void> p3;

  for (size_t i = 0; i < 10; ++i) {
    managerA->launch([&p1, &p2] {
      p2.set_value();
      p1.get_future().get();
      p1 = std::promise<void>();
    });
    managerB->launch([&p2, &p3] {
      p2.get_future().get();
      p2 = std::promise<void>();
      p3.set_value();
    });
    managerC->launch([&p3, &p1] {
      p3.get_future().get();
      p3 = std::promise<void>();
      p1.set_value();
    });
  }

  EXPECT_EQ(std::future_status::ready, managerA->stop().wait_for(Async::kTestTimeout));
  EXPECT_EQ(std::future_status::ready, managerB->stop().wait_for(Async::kTestTimeout));
  EXPECT_EQ(std::future_status::ready, managerC->stop().wait_for(Async::kTestTimeout));
}

} /* namespace Task */
