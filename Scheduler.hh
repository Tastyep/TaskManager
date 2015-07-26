#ifndef SCHEDULER_HH_
#define SCHEDULER_HH_

#include "ThreadManager.hh"
#include "Task.hpp"

class Scheduler {
public:
  Scheduler(unsigned int nbThreads,
            ThreadManager& manager);

  virtual ~Scheduler();

  template<class F, class... Args>
  auto runAt(F&& function, const std::chrono::steady_clock::time_point& timePoint, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    std::future<return_type> futureResult;

    if (this->status.load(std::memory_order_release) == state::STOP)
      throw std::runtime_error("Can't add task on stopped Scheduler");

    auto packagedTask = std::make_shared<std::packaged_task<return_type()> >
    (std::bind(std::forward<F>(function), std::forward<Args>(args)...));
    futureResult = packagedTask->get_future();

    this->runAt([this, packagedTask]() {
        (*packagedTask)();
      }, timePoint);
    return futureResult;
  }

  void runAt(const Task& task,
             const std::chrono::steady_clock::time_point& timePoint);

  template<class F, class... Args>
  void runEvery(F&& function, const std::chrono::steady_clock::duration& duration, Args&&... args) {
    if (this->status.load(std::memory_order_release) == state::STOP)
      throw std::runtime_error("Can't add task on stopped Scheduler");
    auto task = std::bind(std::forward<F>(function), std::forward<Args>(args)...);
    auto cuNow = std::chrono::steady_clock::now() + duration;
    this->runAt([this, task, duration]() {
        task();
        auto now = std::chrono::steady_clock::now();
        this->runAt(task, now + duration);
      }, cuNow);
  }

public:
  std::pair<bool, std::string> start();
  std::pair<bool, std::string> pause();
  std::pair<bool, std::string> unpause();
  std::pair<bool, std::string> stop();

private:
  std::atomic<state> 	status;
  std::vector<std::pair<Task, std::chrono::steady_clock::time_point> > taskContainer;
  std::mutex taskMutex;
};

#endif /* end of include guard: SCHEDULER_HH_ */
