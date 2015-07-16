#ifndef WORKER_HH_
#define WORKER_HH_

#include "Task.hpp"

enum class state {
  STOP = 0,
  PAUSE,
  START
};

class Worker {
public:
  Worker();
  ~Worker();

  void start(std::condition_variable& cv,
            std::mutex& condvarMutex);
  void stop();
  void waitStopped();
  void setTask(const std::function<void ()>& task);
  void setTask(const Task& task);
  Task& getTask();
  bool isIdle();

private:
  void threadMain(std::condition_variable& cv, std::mutex& condvarMutex);

private:
  Task  task;
  std::thread thread;
  std::mutex  mutex;
  bool running;
};

#endif /* end of include guard: WORKER_HH_ */
