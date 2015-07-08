#include "ThreadManager.hh"

ThreadManager::ThreadManager(unsigned int nbThread) : running(true)
{
  for (unsigned int i = 0; i < nbThread; ++i) {
    this->addNewThread();
  }
}

ThreadManager::~ThreadManager() {
  if (this->running)
    this->stop();
  this->cv.notify_all();
  {
    std::lock_guard<std::mutex> guard(this->workersMutex);

    for (auto& worker : this->workers) {
      if (worker.thread.joinable())
        worker.thread.join();
    }
  }
}

void
ThreadManager::addNewThread() {
  std::lock_guard<std::mutex> guard(this->workersMutex);

  this->workers.emplace_back([this](Worker *worker) {
    while (this->running) {
      {
        std::unique_lock<std::mutex> lock(this->condvarMutex);
        do {
          this->cv.wait(lock, [this, worker] {
            return (not this->running || worker->task != nullptr);
          });
        } while (this->running && worker->task == nullptr);
        if (not this->running)
          return ;
      }
      //do some stuff
    }
  });
}

unsigned int
ThreadManager::roundToNextPower(unsigned int nbThread) const {
  nbThread |= nbThread >> 1;
  nbThread |= nbThread >> 2;
  nbThread |= nbThread >> 4;
  nbThread |= nbThread >> 8;
  nbThread |= nbThread >> 16;
  ++nbThread;
  return nbThread;
}

void
ThreadManager::runTask(const std::function<void ()>& task) {
  unsigned int maxThreads;
  unsigned int nbWorkers;

  if (not this->running)
    return ;
  {
    std::lock_guard<std::mutex> guard(this->workersMutex);

    for (auto& worker : this->workers) {
      if (worker.task == nullptr) {
        worker.task = task;
        return ;
      }
    }
  }
  {
    std::lock_guard<std::mutex> guard(this->workersMutex);
    nbWorkers = this->workers.size();
    maxThreads = roundToNextPower(nbWorkers);
  }
  for (unsigned int i = 0; i < maxThreads - nbWorkers; ++i) {
    this->addNewThread();
  }
  this->runTask(task);
}

std::pair<bool, std::string>
ThreadManager::stop() {
  if (this->running == false) {
      return std::make_pair(false, "ThreadManager is already stopped");
  }
  this->running = false;
  return std::make_pair(true, "");
};
