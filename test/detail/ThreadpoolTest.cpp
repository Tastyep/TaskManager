#include "test/detail/ThreadpoolTest.hh"

namespace Task {
namespace Detail {

TEST_F(DetailThreadpool, ScheduleOne) {
  this->schedule({
    std::chrono::milliseconds(1),
  });
}

TEST_F(DetailThreadpool, ScheduleMultiOrdered) {
  this->schedule({
    std::chrono::milliseconds(1),
    std::chrono::milliseconds(2),
    std::chrono::milliseconds(3),
  });
}

TEST_F(DetailThreadpool, ScheduleMultiSameTime) {
  this->schedule({
    std::chrono::milliseconds(1),
    std::chrono::milliseconds(1),
    std::chrono::milliseconds(1),
  });
}

TEST_F(DetailThreadpool, ScheduleReverseOrder) {
  this->schedule({
    std::chrono::milliseconds(3),
    std::chrono::milliseconds(2),
    std::chrono::milliseconds(1),
  });
}

TEST_F(DetailThreadpool, ScheduleArbitraryOrder) {
  this->schedule({
    std::chrono::milliseconds(2),
    std::chrono::milliseconds(1),
    std::chrono::milliseconds(3),
  });
}

TEST_F(DetailThreadpool, UseThreadpoolFromTask) {
  auto promise = std::make_shared<std::promise<void>>();
  auto future = promise->get_future();

  auto task = [this, promise = std::move(promise)] {
    auto task = [promise = std::move(promise)] {
      promise->set_value();
    };
    _threadpool.schedule(TimedTask{ std::move(task), Clock::now() });
  };
  _threadpool.schedule(TimedTask{ std::move(task), Clock::now() });

  EXPECT_EQ(std::future_status::ready, future.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
}

} /* namespace Detail */
} /* namespace Task */
