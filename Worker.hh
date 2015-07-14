#ifndef WORKER_HH_
#define WORKER_HH_

#include "Task.hpp"

class Worker {
public:
  Worker();
  ~Worker();

  void start(std::condition_variable& cv,
            std::mutex& condvarMutex);
  void stop();
  void setTask(const std::function<void ()>& task);
  bool isIdle();

private:
  void threadMain(std::condition_variable& cv, std::mutex& condvarMutex);

private:
  Task   task;
  std::thread thread;
  std::mutex  mutex;
  bool running;
};

#endif /* end of include guard: WORKER_HH_ */
