#include "test/SchedulerTest.hh"

#include <atomic>

using namespace std::chrono_literals;

namespace Task {

TEST_F(SchedulerTest, scheduleOne) {
  this->setup(1, 1);
  this->runTasks(); // This makes a task for blocking the scheduler.
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
       EXPECT_EQ(std::future_status::ready, f1.wait_for(Async::kTestTimeout));
     },
      0us },
    { [&p1, &f2] {
       p1.set_value();
       EXPECT_EQ(std::future_status::ready, f2.wait_for(Async::kTestTimeout));
     },
      0us },
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
      0us },
    { [&p1, &f2] {
       p1.set_value();
       EXPECT_EQ(std::future_status::ready, f2.wait_for(Async::kTestTimeout));
     },
      0us },
  });
  this->runTasks();
}

TEST_F(SchedulerTest, scheduleNested) {
  this->setup(2, 2);
  this->addTasks({
    { [this] {
       std::promise<void> promise;
       auto future = promise.get_future();

       _scheduler->scheduleIn("0", 0us, [promise = std::move(promise)]() mutable { promise.set_value(); });

       EXPECT_EQ(std::future_status::ready, future.wait_for(Async::kTestTimeout));
     },
      0us },
  });

  this->runTasks();
}

TEST_F(SchedulerTest, schedulePeriodic) {
  auto threadpool = std::make_shared<Detail::Threadpool>(2);
  auto scheduler = std::make_shared<Scheduler>(threadpool, 1);
  std::promise<void> p;
  auto f = p.get_future();
  size_t i = 0;

  scheduler->scheduleEvery("0", std::chrono::milliseconds(0), [&i, &p] {
    ++i;
    if (i == 5) {
      p.set_value();
    }
  });

  EXPECT_EQ(std::future_status::ready, f.wait_for(Async::kTestTimeout));
  EXPECT_EQ(std::future_status::ready, scheduler->stop().wait_for(Async::kTestTimeout));
}

TEST_F(SchedulerTest, multiplePeriodicTasksOneWorker) {
  _threadpool = std::make_shared<Detail::Threadpool>(2);
  auto scheduler = this->setupScheduler(1);
  std::promise<void> p;
  auto f = p.get_future();
  std::string strA = "HloWrd";
  std::string strB = "el ol!";
  std::string expected = "Hello World!";
  std::string final;
  size_t i = 0;

  scheduler->scheduleEvery("A", std::chrono::milliseconds(0), [&] { //
    if (i < strA.size()) {
      final.push_back(strA[i]);
    }
  });
  scheduler->scheduleEvery("B", std::chrono::milliseconds(0), [&] { //
    if (i < strB.size()) {
      final.push_back(strB[i++]);
      if (i == strB.size()) {
        p.set_value();
      }
    }
  });
  // Unblock the scheduler.
  this->runTasks();
  EXPECT_EQ(std::future_status::ready, f.wait_for(Async::kTestTimeout));

  EXPECT_EQ(expected, final);
  EXPECT_EQ(std::future_status::ready, scheduler->stop().wait_for(Async::kTestTimeout));
}

TEST_F(SchedulerTest, erase) {
  size_t n = 0;

  this->setup(1, 1);
  this->addTasks({
    { [&n] { ++n; }, 0us },
    { [&n] { ++n; }, 1us },
    { [&n] { ++n; }, 2us },
    { [&n] { ++n; }, 3us },
    { [&n] { ++n; }, 4us },
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
    { [] {}, 0us },
  });
  EXPECT_TRUE(_scheduler->isScheduled("0"));
  this->runTasks();
}

TEST_F(SchedulerTest, stopAndDiscard) {
  size_t n = 0;

  this->setup(1, 1);
  this->addTasks({
    { [&n] { n = 1; }, 1min },
  });
  auto done = _scheduler->stop(true);

  this->runTasks();
  EXPECT_EQ(std::future_status::ready, done.wait_for(Async::kTestTimeout));
  EXPECT_EQ(0, n);
}

TEST_F(SchedulerTest, scheduleOnStopped) {
  auto threadpool = std::make_shared<Detail::Threadpool>(2);
  auto scheduler = std::make_shared<Scheduler>(threadpool, 1);

  EXPECT_EQ(std::future_status::ready, scheduler->stop().wait_for(Async::kTestTimeout));
  auto future = scheduler->scheduleIn("0", 0us, [] {});

  EXPECT_FALSE(scheduler->isScheduled("0"));
  EXPECT_TRUE(future.valid());
  EXPECT_THROW(future.get(), std::future_error);
}

} /* namespace Task */
