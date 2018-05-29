#ifndef TASK_MANAGER_MANAGER_HPP
#define TASK_MANAGER_MANAGER_HPP

#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <utility>

#include "TaskManager/detail/Task.hpp"
#include "TaskManager/detail/Threadpool.hpp"

namespace Task {

//! The task manager.
class Manager {
 public:
  //! Task manager constructor.
  //! @param threadpool The threadpool owning the workers.
  //! @param maxWorkers The maximum number of parallel executions.
  Manager(std::shared_ptr<Detail::Threadpool> threadpool, size_t maxWorkers);

  //! Synchronize and stop the task manager.
  //! @return A future that signals when the manager can be destroyed.
  std::future<void> stop();

  //! Add a new task to the manager.
  //! @param function The function to execute.
  //! @param args the parameters to pass to the function.
  template <class F, class... Args>
  auto push(F&& function, Args&&... args) //
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using ReturnType = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>( //
      std::bind(std::forward<F>(function), std::forward<Args>(args)...));
    auto future = task->get_future();

    auto functor = [this, task = std::move(task)]() mutable {
      (*task)();
      {
        std::lock_guard<std::mutex> guard(_mutex);

        --_workerCount;
        this->processTasks();
      }
    };
    this->addTask(std::move(functor));

    return future;
  }

 private:
  void addTask(std::function<void()> functor);
  void processTasks();

 private:
  std::shared_ptr<Detail::Threadpool> _threadpool;
  std::queue<Detail::TimedTask> _tasks;
  std::mutex _mutex;
  size_t _maxWorkers;
  size_t _workerCount{ 0 };
  bool _stopped{ false };
};

} /* namespace Task */

#endif
