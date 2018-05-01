#include "detail/Threadpool.hh"

namespace Task {
namespace Detail {

Threadpool::Threadpool(size_t threadCount) {
  _workers.reserve(threadCount);
  for (size_t i = 0; i < threadCount; ++i) {
    _workers.emplace_back(&Threadpool::processTasks, this);
  }
}

Threadpool::~Threadpool() {
  {
    std::lock_guard<std::mutex> guard(_mutex);

    _stopRequested = true;
  }
  _cv.notify_all();
  for (auto& worker : _workers) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void Threadpool::schedule(TimedTask task) {
  {
    std::lock_guard<std::mutex> guard(_mutex);

    _tasks.push(std::move(task));
  }
  _cv.notify_one();
}

void Threadpool::processTasks() {
  bool stopped = false;

  while (!stopped) {
    std::unique_lock<std::mutex> lock(_mutex);
    auto timeout = Clock::now();
    bool taskReady = false;

    while (!taskReady) {
      taskReady = _cv.wait_until(lock, timeout, [this, &timeout, &stopped] { //
        stopped = _stopRequested;
        if (stopped) {
          return true;
        }
        // The condvar can be unblocked by calling notify_xxx to signal that a new task has been added.
        // We should update the timeout if the new task has a higher or lower priority.
        if (!_tasks.empty()) {
          timeout = _tasks.top().timepoint();
          return timeout <= Clock::now();
        }
        timeout = Clock::now() + std::chrono::hours(1);

        return false;
      });
      if (stopped) {
        return;
      }
    }

    auto task = _tasks.top();
    _tasks.pop();

    lock.unlock();
    task();
  }
}

} /* namespace Detail */
} /* namespace Task */
