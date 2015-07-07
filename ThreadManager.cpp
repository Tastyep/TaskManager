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
  for (auto& worker : this->threads) {
    if (worker.thread.joinable())
      worker.thread.join();
  }
}

void
ThreadManager::addNewThread() {
  this->threads.emplace_back([this](Worker *worker) {
    while (this->running) {
      {
        std::unique_lock<std::mutex> lock(this->condvarMutex);
        do {
          this->cv.wait(lock, [this, worker] {
            return (not this->running || worker->status == Worker::state::Idle);
          });
        } while (this->running && worker->status == Worker::state::Idle);
        if (not this->running)
          return ;
      }
      //do some stuff
    }
  });
}

std::pair<bool, std::string>
ThreadManager::stop() {
  if (this->running == false) {
      return std::make_pair(false, "ThreadManager is already stopped");
  }
  this->running = false;
  return std::make_pair(true, "");
};
