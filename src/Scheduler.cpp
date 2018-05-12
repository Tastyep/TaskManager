#include "Scheduler.hh"

#include <iostream>

namespace Task {

Scheduler::Scheduler(std::shared_ptr<Detail::Threadpool> threadpool, size_t maxWorkers)
  : _threadpool(std::move(threadpool))
  , _maxWorkers(maxWorkers) {}

std::future<void> Scheduler::stop(bool discard) {
  std::lock_guard<std::mutex> guard(_mutex);

  _stopped = true;
  if (discard) {
    _tasks.clear();
  }
  auto task = std::make_shared<std::packaged_task<void()>>([this] {
    std::unique_lock<std::mutex> guard(_mutex);
    bool isLast = _workerCount == 0;

    // Guarantee that the task finishes last.
    while (!isLast) {
      guard.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      guard.lock();
      isLast = _workerCount == 0;
    }
  });
  auto future = task->get_future();
  Detail::Timepoint timepoint = Detail::Clock::now();

  if (!_tasks.empty()) {
    // Set the task just after the last one.
    timepoint = _tasks.top().timepoint() + std::chrono::nanoseconds(1);
  }
  auto functor = [task = std::move(task)]() mutable {
    (*task)();
  };
  // Push the task directly into the pool to avoid id collisions.
  _threadpool->execute({ std::move(functor), timepoint });

  return future;
}

void Scheduler::remove(const std::string& id) {
  std::lock_guard<std::mutex> guard(_mutex);

  _tasks.erase(TimedTask(_hasher(id)));
}

bool Scheduler::isScheduled(const std::string& id) const {
  std::lock_guard<std::mutex> guard(_mutex);

  return _tasks.contain(TimedTask(_hasher(id)));
}

// Private methods

void Scheduler::addTask(const std::string& id, std::function<void()> functor, Detail::Timepoint timepoint) {
  std::lock_guard<std::mutex> guard(_mutex);

  if (_stopped) {
    return;
  }
  _tasks.emplace(_hasher(id), std::move(functor), timepoint);
  this->processTasks();
}

void Scheduler::processTasks() {
  if (_tasks.empty() || _workerCount == _maxWorkers) {
    return;
  }
  auto task = std::move(_tasks.top());
  _tasks.pop();

  ++_workerCount;
  _threadpool->execute(task);
}

} /* namespace Task */
