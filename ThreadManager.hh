#ifndef THREADMANAGER_HH_
#define THREADMANAGER_HH_

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <utility>
#include <string>

class Worker {
public:
  enum class state {
    Idle = 0,
    Working
  };

  Worker(const std::function<void (Worker *)>& func) : thread(func, this), status(state::Idle) {
  }
  Worker(Worker&& data) :
  thread(std::move(data.thread))
  , status(std::move(data.status.load()))
  , task(std::move(data.task)) {}

  std::thread thread;
  std::atomic<state> status;
  std::function<void ()> task;
};

class ThreadManager {
public:
  ThreadManager(unsigned int nbThread = 1);
  ~ThreadManager();

  std::pair<bool, std::string> stop();

private:
  void addNewThread();

private:
  std::vector<Worker> threads;
  std::condition_variable cv;
  std::mutex condvarMutex;
  bool running;
};

#endif /* end of include guard: THREADMANAGER_HH_ */
