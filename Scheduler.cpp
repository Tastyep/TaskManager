#include <tuple>
#include <iterator>
#include "Scheduler.hh"

Scheduler::Scheduler(unsigned int nbThreads,
                     ThreadManager& manager) :
threadRefCount(0)
, maxParallelism(nbThreads)
, manager(manager)
, status(state::START)
{
  std::shared_ptr<Worker> worker = manager.getWorker();
  Task task;

  task.assign(&Scheduler::mainFunction, this);
}

Scheduler::~Scheduler() {
  if (this->status.load(std::memory_order_seq_cst) != state::STOP) {
    this->stop();
  }
}

void
Scheduler::mainFunction() {
  std::pair<Task, std::chrono::steady_clock::time_point> task;
  auto time_wait = std::chrono::steady_clock::now();
  std::shared_ptr<Worker> worker;
  while (this->status.load() != state::STOP) {
    std::unique_lock<std::mutex> lock(this->condvarMutex);

    this->cv.wait_until(lock, time_wait, [this, &task, &time_wait, &worker]() {
      if (this->status.load() == state::STOP)
        return true;
      {
        std::lock_guard<std::mutex> guard(this->refCountMutex);

        if (this->threadRefCount >= this->maxParallelism) {
          time_wait = std::chrono::steady_clock::now() + std::chrono::hours(1);
          return false;
        }
        task = this->getHighestPriorityTask();
        if (task.first == nullptr) { // update time to wait
          time_wait = task.second;
          return false;
        }

        worker = manager.getWorker();
        ++this->threadRefCount;

        std::lock_guard<std::mutex> guardWorker(this->workerMutex);
        this->workers.emplace_back(worker);
        task.first.addCallback([this, worker] {
          this->removeWorkerRef(worker);
          this->decreaseRefCount();
          this->cv.notify_all();
        });
      }
      return true;
    });
    manager.startTask(worker, task.first);
  }
}

void
Scheduler::removeWorkerRef(std::shared_ptr<Worker> worker) {
  std::lock_guard<std::mutex> guardWorker(this->workerMutex);
  this->workers.erase(std::remove_if(this->workers.begin(), this->workers.end(),
  [worker](const auto& w) { return w == worker; }), this->workers.end());
}

void
Scheduler::decreaseRefCount() {
  std::lock_guard<std::mutex> guardRef(this->refCountMutex);
  --(this->threadRefCount);
}

std::pair<Task, std::chrono::steady_clock::time_point>
Scheduler::getHighestPriorityTask() {
    std::lock_guard<std::mutex> guard(this->taskMutex);
  Task task;
  bool updateTask = true;

  if (this->taskContainer.empty())
    return std::make_pair(task, std::chrono::steady_clock::now() + std::chrono::hours(1)); // Arbitrary value, just wait until it get's notified
  auto saveIt = this->taskContainer.begin();
  auto& tp = saveIt->second;

  for (auto it = std::next(this->taskContainer.begin()); it != this->taskContainer.end(); ++it) {
    if (it->second < tp) {
      tp = it->second;
      saveIt = it;
    }
  }
  for (auto it = this->uniqueTask.begin(); it != uniqueTask.end(); ++it) {
    if (it->second < tp) {
      tp = it->second;
      saveIt = it;
      updateTask = false;
    }
  }
  auto now = std::chrono::steady_clock::now();
  if (saveIt->second > now)
    return std::make_pair(task, saveIt->second);
  if (updateTask) {
    saveIt->second = now;
    task = saveIt->first;
  }
  else {
    task = std::move(saveIt->first);
    this->uniqueTask.erase(saveIt);
  }
  return std::make_pair(task, now);
}

void
Scheduler::runAt(const Task& task,
                 const std::chrono::steady_clock::time_point& timePoint) {
   if (this->status.load(std::memory_order_release) == state::STOP)
     throw std::runtime_error("Can't add task on stopped Scheduler");
   std::lock_guard<std::mutex> guard(this->taskMutex);
   this->taskContainer.emplace_back(task, timePoint);
}

void
Scheduler::runEvery(const Task& task,
                    const std::chrono::steady_clock::duration& duration) {
  auto cuNow = std::chrono::steady_clock::now() + duration;
  this->runAt(Task([this, task(task), duration]() mutable {
      task();
      if (this->status.load(std::memory_order_release) != state::STOP) {
        auto now = std::chrono::steady_clock::now();
        this->runAt(task, now + duration);
      }
    }), cuNow);
}

std::pair<bool, std::string>
Scheduler::start() {
  if (this->status.load(std::memory_order_seq_cst) != state::STOP) {
      return std::make_pair(false, "Scheduler has already been started");
  }

  status.store(state::START, std::memory_order_seq_cst);       // we can now exectue tasks
  return std::make_pair(true, "");
}

std::pair<bool, std::string>
Scheduler::pause() {
  if (this->status.load(std::memory_order_seq_cst) != state::START) {
      return std::make_pair(false, "Scheduler is not started");
  }
  this->status.store(state::PAUSE, std::memory_order_acquire);

  std::lock_guard<std::mutex> guardWorker(this->workerMutex);
  for (auto& worker : this->workers) {
    worker->pauseTask();
  }
  return std::make_pair(true, "");
}

std::pair<bool, std::string>
Scheduler::unpause() {
  if (this->status.load(std::memory_order_seq_cst) != state::PAUSE) {
      return std::make_pair(false, "Scheduler is not paused");
  }
  status.store(state::START, std::memory_order_acquire);

  {
    std::lock_guard<std::mutex> guardWorker(this->workerMutex);
    for (auto& worker : this->workers) {
      worker->unpauseTask();
    }
  }
  return std::make_pair(true, "");
}

std::pair<bool, std::string>
Scheduler::stop() {
  bool waitCondition;
  if (this->status.load(std::memory_order_seq_cst) == state::STOP) {
      return std::make_pair(false, "Scheduler is already stopped");
  }
  this->status.store(state::STOP, std::memory_order_acquire);

  {
    std::lock_guard<std::mutex> guardWorker(this->workerMutex);
    for (auto& worker : this->workers) {
      worker->stopTask();
    }
  }
  do {
    std::lock_guard<std::mutex> guardWorker(this->workerMutex);
    waitCondition = (not this->workers.empty() || not this->taskContainer.empty());
    if (waitCondition)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
  } while (waitCondition);
  return std::make_pair(true, "");
};
