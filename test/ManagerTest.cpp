#include "test/ManagerTest.hh"

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
      EXPECT_EQ(std::future_status::ready, f1.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
    },
    [&p1, &f2] {
      p1.set_value();
      EXPECT_EQ(std::future_status::ready, f2.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
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
      EXPECT_EQ(std::future_status::timeout, f1.wait_for(std::chrono::milliseconds(1)));
    },
    [&p1, &f2] {
      p1.set_value();
      EXPECT_EQ(std::future_status::ready, f2.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
    },
  });
  this->runTasks();
}

} /* namespace Task */
