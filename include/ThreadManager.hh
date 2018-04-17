#ifndef THREADMANAGER_HH_
#define THREADMANAGER_HH_

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include "Worker.hh"

namespace TaskManager {

class ThreadManager {
 public:
  ThreadManager(unsigned int nbThread = std::thread::hardware_concurrency());
  ~ThreadManager();

  std::pair<bool, std::string> stop();
  std::shared_ptr<Worker> getWorker();
  void startTask(std::shared_ptr<Worker> worker, const Task& task);

 private:
  unsigned int roundToNextPower(unsigned int nbThread) const;
  void addNewThread();

 private:
  std::vector<std::shared_ptr<Worker>> workers;
  std::mutex workersMutex;

  mutable std::condition_variable cv;
  mutable std::mutex condvarMutex;

  std::atomic_bool running;
};
} // namespace TaskManager

#endif /* end of include guard: THREADMANAGER_HH_ */
