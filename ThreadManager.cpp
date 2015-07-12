#include "ThreadManager.hh"
#include <iostream>
#include <chrono>

ThreadManager::ThreadManager(unsigned int nbThread) : running(true)
{
  for (unsigned int i = 0; i < nbThread; ++i) {
    this->addNewThread();
  }
}

ThreadManager::~ThreadManager() {
  if (this->running)
    this->stop();
  {
    std::lock_guard<std::mutex> guard(this->workersMutex);
    workers.clear();
  }
}

void
ThreadManager::addNewThread() {
  std::lock_guard<std::mutex> guard(this->workersMutex);

  this->workers.emplace_back(new Worker);
  this->workers.back()->start(this->cv, this->condvarMutex);
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
      if (worker->isIdle()) {
        worker->setTask(task);
        this->cv.notify_all();
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
  std::lock_guard<std::mutex> guard(this->workersMutex);

  if (this->running == false) {
    return std::make_pair(false, "ThreadManager is already stopped");
  }
  this->running = false;
  for (auto& worker : this->workers)
    worker->stop();
  this->cv.notify_all();
  return std::make_pair(true, "");
};
