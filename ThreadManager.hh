#ifndef THREADMANAGER_HH_
#define THREADMANAGER_HH_

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <condition_variable>
#include <vector>
#include <utility>
#include <string>
#include <iostream>

class Worker {
public:
  Worker() :
  thread()
  , task(nullptr)
  , running(false) {
  }

  ~Worker() {
    if (this->running)
      this->stop();
    if (this->thread.joinable())
      this->thread.join();
  };

  void
  start(std::condition_variable& cv,
        std::mutex& condvarMutex) {
    this->running = true;
    this->thread = std::thread(&Worker::threadMain, this, std::ref(cv), std::ref(condvarMutex));
  }

  void
  stop() {
    this->running = false;
  }

  void
  threadMain(std::condition_variable& cv, std::mutex& condvarMutex) {
    while (this->running) {
      {
        std::unique_lock<std::mutex> lock(condvarMutex);

        cv.wait(lock, [this] {
          return (not this->running || this->task != nullptr);
        });
      }
      if (not this->running)
        return ;
      this->task();
      this->setTask(nullptr);
    }
  }

  void
  setTask(const std::function<void ()>& task) {
    std::lock_guard<std::mutex> guard(this->mutex);
    this->task = task;
  }

  bool
  isIdle() const {
    return (this->task == nullptr);
  }

public:
    std::function<void ()> task;

private:
  std::thread thread;
  std::mutex mutex;
  bool running;
};

class ThreadManager {
public:
  ThreadManager(unsigned int nbThread = std::thread::hardware_concurrency());
  ~ThreadManager();

  std::pair<bool, std::string> stop();
  void runTask(const std::function<void ()>& task);

private:
  unsigned int roundToNextPower(unsigned int nbThread) const;
  void addNewThread();

private:
  std::vector<std::unique_ptr<Worker>> workers;
  std::mutex workersMutex;

  mutable std::condition_variable cv;
  mutable std::mutex condvarMutex;

  bool running;
};

#endif /* end of include guard: THREADMANAGER_HH_ */
