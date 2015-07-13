#include "ThreadPool.hh"

ThreadPool::ThreadPool(unsigned int nbThreads,
                ThreadManager& manager) :
threadRefCount(0)
, manager(manager)
, status(state::STOP)
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
  return std::make_pair(true, "");
}

std::pair<bool, std::string>
ThreadPool::unpause() {
  if (this->status.load(std::memory_order_seq_cst) != state::PAUSE) {
      return std::make_pair(false, "ThreadPool is not paused");
  }
  status.store(state::START, std::memory_order_acquire);
  this->startTask();
  return std::make_pair(true, "");
}

std::pair<bool, std::string>
ThreadPool::stop() {
  if (this->status.load(std::memory_order_seq_cst) == state::STOP) {
      return std::make_pair(false, "ThreadPool is already stopped");
  }
  this->status.store(state::STOP, std::memory_order_acquire);
  return std::make_pair(true, "");
};

void
ThreadPool::addTask(Task& task) {
  if (this->status.load(std::memory_order_release) == state::STOP)
    throw std::runtime_error("Can't add task on stopped ThreadPool");

  task >> [this]() {
    {
      std::lock_guard<std::mutex> guardRef(this->refCountMutex);
      --(this->threadRefCount);
    }
    this->startTask();
  };
  {
    std::lock_guard<std::mutex> guard(this->taskMutex);
    taskContainer.push(task);
  }
  this->startTask();
}
