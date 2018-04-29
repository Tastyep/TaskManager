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
  _stopRequested = true;
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
  while (!_stopRequested) {
    std::unique_lock<std::mutex> lock(_mutex);
    auto timeout = Clock::now() + std::chrono::hours(1);
    bool taskReady = false;

    while (!taskReady) {
      // The condvar can be unblocked by calling notify_xxx to signal that a new task has been added.
      // We should update the timeout if the new task has a higher priority.
      if (!_tasks.empty()) {
        timeout = _tasks.top().timepoint();
      }
      taskReady = _cv.wait_until(lock, timeout, [this] { //
        return !_tasks.empty() || _stopRequested;
      });
      if (_stopRequested) {
        return;
      }
      // This is done to avoid executing the most recent task when another one gets added.
      // Still, it is necessary to wake up the thread in order to update the timeout.
      // The container's size needs to be checked as the condvar is not garanted to obtain the lock first.
      if (_tasks.empty() || _tasks.top().timepoint() > Clock::now()) {
        taskReady = false;
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
