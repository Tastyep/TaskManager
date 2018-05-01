#ifndef TASK_DETAIL_THREAD_POOL_TEST_HH
#define TASK_DETAIL_THREAD_POOL_TEST_HH

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <future>

#include "detail/Threadpool.hh"

#include "test/Async.hh"

using testing::Test;

namespace Task {
namespace Detail {

class DetailThreadpool : public Test {
 public:
  DetailThreadpool()
    : _threadpool(1) {}
  ~DetailThreadpool() = default;

  // Synchronize ensures that the scheduler blocks on the first task while we set up our test.
  std::shared_ptr<std::promise<void>> synchronize() {
    auto lock = std::make_shared<std::promise<void>>();
    std::shared_future<void> future = lock->get_future();

    auto task = [future = std::move(future)] {
      EXPECT_EQ(std::future_status::ready, future.wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
    };
    _threadpool.execute(TimedTask{ std::move(task), Clock::now() - std::chrono::hours(1) }); // arbitrary value.

    return lock;
  }

  void schedule(const std::vector<std::chrono::milliseconds>& delays) {
    std::vector<std::future<void>> futures;
    std::vector<std::chrono::milliseconds> measuredDelay(delays.size());
    std::vector<size_t> ids;
    const auto startTime = Clock::now();

    auto lock = this->synchronize();
    for (size_t i = 0; i < delays.size(); ++i) {
      auto promise = std::make_shared<std::promise<void>>();
      futures.push_back(promise->get_future());

      auto task = [&measuredDelay, &ids, promise = std::move(promise), startTime, i] {
        const auto takenTime = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - startTime);

        measuredDelay[i] = takenTime;
        ids.push_back(i);
        promise->set_value();
      };
      _threadpool.execute(TimedTask{ std::move(task), startTime + delays[i] });
    }

    lock->set_value();
    for (size_t i = 0; i < futures.size(); ++i) {
      EXPECT_EQ(std::future_status::ready, futures[i].wait_for(std::chrono::milliseconds(Async::kTestTimeout)));
      EXPECT_TRUE(measuredDelay[i] >= delays[i]);
    }

    auto orderedDelays = delays;
    std::sort(orderedDelays.begin(), orderedDelays.end(), std::greater<>());
    for (size_t i = 0; i < ids.size(); ++i) {
      const auto id = ids[i];
      EXPECT_EQ(delays[id], orderedDelays.back());
      orderedDelays.pop_back();
    }
  }

 public:
  Threadpool _threadpool;
};

} /* namespace Detail */
} /* namespace Task */

#endif
