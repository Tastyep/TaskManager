#include "Manager.hh"

namespace Task {

Manager::Manager(std::shared_ptr<Detail::Threadpool> threadpool, size_t maxWorkers)
  : _threadpool(std::move(threadpool))
  , _maxWorkers(maxWorkers) {}

std::future<void> Manager::stop() {
  auto task = std::make_shared<std::packaged_task<void()>>([this] {
    std::unique_lock<std::mutex> guard(_mutex);
    bool isLast = _workerCount == 1;

    // Guarantee that the task finishes last.
    while (!isLast) {
      guard.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      guard.lock();
      isLast = _workerCount == 1;
    }
  });
  auto future = task->get_future();

  // Adding a new task and expecting the future guarantees that the last batch of tasks is being executed.
  auto functor = [task = std::move(task)]() mutable {
    (*task)();
  };
  std::lock_guard<std::mutex> guard(_mutex);

  _stopped = true;
  _tasks.emplace(std::move(functor), Detail::Clock::now());
  this->processTasks();

  return future;
}

void Manager::addTask(std::function<void()> functor) {
  std::lock_guard<std::mutex> guard(_mutex);

  if (_stopped) {
    return;
  }
  _tasks.emplace(std::move(functor), Detail::Clock::now());
  this->processTasks();
}

void Manager::processTasks() {
  if (_tasks.empty() || _workerCount == _maxWorkers) {
    return;
  }
  auto task = std::move(_tasks.front());
  _tasks.pop();

  ++_workerCount;
  _threadpool->execute(std::move(task));
}

} /* namespace Task */
