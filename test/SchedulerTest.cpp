#include "test/SchedulerTest.hh"

#include <atomic>

namespace Task {

TEST_F(SchedulerTest, scheduleOne) {
  this->setup(1, 1);
  this->runTasks(); // This makes a task for blocking the manager.
}

TEST_F(SchedulerTest, scheduleMultipleOrderedSequentially) {
  size_t n = 0;

  this->setup(1, 1);
  for (size_t i = 0; i < 10; ++i) {
    this->addTasks({
      { [&n, i] { n = i; }, std::chrono::microseconds(i) },
    });
  }
  this->runTasks();

  EXPECT_EQ(9, n);
}

TEST_F(SchedulerTest, scheduleMultipleUnOrderedSequentially) {
  size_t n = 0;

  this->setup(1, 1);
  for (size_t i = 10; i > 0; --i) {
    this->addTasks({
      { [&n, i] { n = i; }, std::chrono::microseconds(i) },
    });
  }
  this->runTasks();

  EXPECT_EQ(1, n);
}

TEST_F(SchedulerTest, scheduleMultipleInParallel) {
  this->setup(2, 2);

  std::vector<std::pair<std::function<void()>, std::chrono::microseconds>> functors;
  for (size_t i = 0; i < 10; ++i) {
    functors.emplace_back([] {}, std::chrono::microseconds(i));
  }
  this->addTasks(std::move(functors));
  this->runTasks();
}

TEST_F(SchedulerTest, scheduleDependentInParallel) {
  std::promise<void> p1;
  std::promise<void> p2;
  auto f1 = p1.get_future();
  auto f2 = p2.get_future();

  this->setup(2, 2);
  this->addTasks({
    { [&p2, &f1] {
       p2.set_value();
       EXPECT_EQ(std::future_status::ready, f1.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
     },
      std::chrono::microseconds(0) },
    { [&p1, &f2] {
       p1.set_value();
       EXPECT_EQ(std::future_status::ready, f2.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
     },
      std::chrono::microseconds(0) },
  });
  this->runTasks();
}

TEST_F(SchedulerTest, scheduleDependentSequentially) {
  std::promise<void> p1;
  std::promise<void> p2;
  auto f1 = p1.get_future();
  auto f2 = p2.get_future();

  this->setup(1, 1);
  this->addTasks({
    { [&p2, &f1] {
       p2.set_value();
       EXPECT_EQ(std::future_status::timeout, f1.wait_for(std::chrono::milliseconds(1)));
     },
      std::chrono::microseconds(0) },
    { [&p1, &f2] {
       p1.set_value();
       EXPECT_EQ(std::future_status::ready, f2.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
     },
      std::chrono::microseconds(0) },
  });
  this->runTasks();
}

TEST_F(SchedulerTest, erase) {
  size_t n = 0;

  this->setup(1, 1);
  this->addTasks({
    { [&n] { ++n; }, std::chrono::microseconds(0) },
    { [&n] { ++n; }, std::chrono::microseconds(1) },
    { [&n] { ++n; }, std::chrono::microseconds(2) },
    { [&n] { ++n; }, std::chrono::microseconds(3) },
    { [&n] { ++n; }, std::chrono::microseconds(4) },
  });
  _scheduler->remove("1");
  _scheduler->remove("4");
  _scheduler->remove("5"); // This task doesn't exist, but it should not cause any error.
  this->runTasks();

  EXPECT_EQ(3, n);
}

TEST_F(SchedulerTest, checkTaskIsScheduled) {
  this->setup(1, 1);
  this->addTasks({
    { [] {}, std::chrono::microseconds(0) },
  });
  EXPECT_TRUE(_scheduler->isScheduled("0"));
  this->runTasks();
}

} /* namespace Task */
