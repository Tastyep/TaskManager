#ifndef THREADMANAGER_HH_
#define THREADMANAGER_HH_

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <utility>
#include <string>

struct Worker {
  Worker(const std::function<void (Worker *)>& func) : thread(func, this) {
  }
  Worker(Worker&& data) :
  thread(std::move(data.thread))
  , task(std::move(data.task)) {
  }

  std::thread thread;
  std::function<void ()> task;
};

class ThreadManager {
public:
  ThreadManager(unsigned int nbThread = 1);
  ~ThreadManager();

  std::pair<bool, std::string> stop();
  void runTask(const std::function<void ()>& task);

private:
  unsigned int roundToNextPower(unsigned int nbThread) const;
  void addNewThread();

private:
  std::vector<Worker> workers;
  std::mutex workersMutex;

  std::condition_variable cv;
  std::mutex condvarMutex;

  bool running;
};

#endif /* end of include guard: THREADMANAGER_HH_ */
