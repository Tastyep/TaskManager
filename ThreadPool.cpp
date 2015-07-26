#include <algorithm>

#include "ThreadPool.hh"

ThreadPool::ThreadPool(unsigned int nbThreads,
                      ThreadManager& manager) :
threadRefCount(0)
, manager(manager)
, status(state::START)
, maxParallelism(nbThreads) {
    if (nbThreads == 0)
      throw std::invalid_argument("The ThreadPool must have at least a thread");
}

ThreadPool::~ThreadPool() {
  if (this->status.load(std::memory_order_seq_cst) != state::STOP) {
    this->stop();
  }
}

std::pair<bool, std::string>
ThreadPool::start() {
  if (this->status.load(std::memory_order_seq_cst) != state::STOP) {
      return std::make_pair(false, "ThreadPool has already been started");
  }

  status.store(state::START, std::memory_order_seq_cst);       // we can now exectue tasks
  return std::make_pair(true, "");
}

std::pair<bool, std::string>
ThreadPool::pause() {
  if (this->status.load(std::memory_order_seq_cst) != state::START) {
      return std::make_pair(false, "ThreadPool is not started");
  }
  this->status.store(state::PAUSE, std::memory_order_acquire);

  std::lock_guard<std::mutex> guardWorker(this->workerMutex);
  for (auto& worker : this->workers) {
    worker->pauseTask();
  }
  return std::make_pair(true, "");
}

std::pair<bool, std::string>
ThreadPool::unpause() {
  if (this->status.load(std::memory_order_seq_cst) != state::PAUSE) {
      return std::make_pair(false, "ThreadPool is not paused");
  }
  status.store(state::START, std::memory_order_acquire);

  {
    std::lock_guard<std::mutex> guardWorker(this->workerMutex);
    for (auto& worker : this->workers) {
      worker->unpauseTask();
    }
  }
  this->startTask();
  return std::make_pair(true, "");
}

std::pair<bool, std::string>
ThreadPool::stop() {
  bool waitCondition;
  if (this->status.load(std::memory_order_seq_cst) == state::STOP) {
      return std::make_pair(false, "ThreadPool is already stopped");
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

void
ThreadPool::resize(unsigned int maxParallelism) {
  std::lock_guard<std::mutex> guardRef(this->refCountMutex);
  this->maxParallelism = maxParallelism;
}

void
ThreadPool::addTask(Task& task) {
  if (this->status.load(std::memory_order_release) == state::STOP)
    throw std::runtime_error("Can't add task on stopped ThreadPool");
  {
    std::lock_guard<std::mutex> guard(this->taskMutex);
    this->taskContainer.push(task);
  }
  this->startTask();
}

void
ThreadPool::decreaseRefCount() {
  {
    std::lock_guard<std::mutex> guardRef(this->refCountMutex);
    --(this->threadRefCount);
  }
}

void
ThreadPool::startTask() {
  std::lock_guard<std::mutex> guardRef(this->refCountMutex);
  std::lock_guard<std::mutex> guardTask(this->taskMutex);
  std::lock_guard<std::mutex> guardWorker(this->workerMutex);
  std::shared_ptr<Worker> worker;
  Task task;

  if (this->taskContainer.empty()
  || this->threadRefCount >= this->maxParallelism
  || this->status.load(std::memory_order_release) == state::PAUSE) // Handle later or already handled
    return ;
  ++this->threadRefCount;
  worker = this->manager.getWorker();
  task = std::move(this->taskContainer.front());
  this->taskContainer.pop();

  this->workers.emplace_back(worker);
  task.addCallback([this, worker] {
    this->removeWorkerRef(worker);
    this->decreaseRefCount();
    this->startTask();
  });

  manager.startTask(worker, task);
  if (this->status.load(std::memory_order_release) == state::STOP)
    task.stop();
}

void
ThreadPool::removeWorkerRef(std::shared_ptr<Worker> worker) {
  std::lock_guard<std::mutex> guardWorker(this->workerMutex);
  this->workers.erase(std::remove_if(this->workers.begin(), this->workers.end(),
  [worker](const auto& w) { return w == worker; }), this->workers.end());
}
