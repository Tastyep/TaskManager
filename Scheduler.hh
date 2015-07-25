#ifndef SCHEDULER_HH_
#define SCHEDULER_HH_

#include "ThreadManager.hh"
#include "TimedTask.hpp"

class Scheduler {
public:
  Scheduler(unsigned int nbThreads,
                  ThreadManager& manager);

  virtual ~Scheduler();

  template<class F, class... Args>
  auto addTask(F&& function, const std::chrono::milliseconds& interval, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    std::future<return_type> futureResult;

    if (this->status.load(std::memory_order_release) == state::STOP)
      throw std::runtime_error("Can't add task on stopped ThreadPool");

    auto packagedTask = std::make_shared<std::packaged_task<return_type()> >
    (std::bind(std::forward<F>(function), std::forward<Args>(args)...));
    futureResult = packagedTask->get_future();

    {
      std::lock_guard<std::mutex> guard(this->taskMutex);
      this->taskContainer.emplace([this, packagedTask]() {
        (*packagedTask)();
      }, interval);
    }
    this->startTask();
    return futureResult;
  }

public:
  std::pair<bool, std::string> start();
  std::pair<bool, std::string> pause();
  std::pair<bool, std::string> unpause();
  std::pair<bool, std::string> stop();

private:
  std::vector<std::pair<TimedTask, std::chrono::milliseconds> > tasks;
  std::mutex taskMutex;
}

#endif /* end of include guard: SCHEDULER_HH_ */
