#ifndef TASK_DETAIL_THREAD_POOL_HH
#define TASK_DETAIL_THREAD_POOL_HH

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "detail/Task.hpp"

namespace Task {
namespace Detail {

class Threadpool {
 public:
  Threadpool(size_t threadCount);
  ~Threadpool();

  void schedule(TimedTask task);

 private:
  void processTasks();

 private:
  std::priority_queue<TimedTask, std::vector<TimedTask>, std::greater<>> _tasks;
  std::vector<std::thread> _workers;
  std::condition_variable _cv;
  std::mutex _mutex;
  std::atomic_bool _stopRequested{ false };
};

} /* namespace Detail */
} /* namespace Task */

#endif
