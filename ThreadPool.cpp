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
  this->emptyTasks();
  return std::make_pair(true, "");
};

void
ThreadPool::emptyTasks() {
  std::lock_guard<std::mutex> guardTask(this->taskMutex);
  Task task;

  while (not this->taskContainer.empty()) {
    task = std::move(this->taskContainer.front());
    this->taskContainer.pop();
    task.abort();
    task();
  }
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
  this->startTask();
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
  || this->status.load(std::memory_order_release) != state::START) // Handle later or already handled
    return ;
  ++this->threadRefCount;
  task = std::move(this->taskContainer.front());
  this->taskContainer.pop();

  task.addCallback([this] { this->decreaseRefCount(); }); // To ensure a new task will be executed
  const auto& stopFunction = task.getStopFunction();
  if (not stopFunction) {
    task.setStopFunction([this, worker]() {
      this->removeWorkerRef(worker);
    });
  }
  else {
    task.setStopFunction([this, stopFunction, worker]() {
      stopFunction();
      this->removeWorkerRef(worker);
    });
  }
  worker = manager.runTask(task); // send worker's state in parameter;
  this->workers.push_back(worker);
}

void
ThreadPool::removeWorkerRef(std::shared_ptr<Worker> worker) {
  std::cout << "Stop" << std::endl;
}
