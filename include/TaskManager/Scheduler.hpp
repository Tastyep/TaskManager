#ifndef TASK_MANAGER_SCHEDULER_HPP
#define TASK_MANAGER_SCHEDULER_HPP

#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "TaskManager/detail/PriorityQueue.hpp"
#include "TaskManager/detail/Task.hpp"
#include "TaskManager/detail/Threadpool.hpp"

namespace Task {

//! The task scheduler.
class Scheduler {
 public:
  //! Task scheduler constructor.
  //! @param threadpool The threadpool owning the workers.
  //! @param maxWorkers The maximum number of parallel executions.
  Scheduler(std::shared_ptr<Detail::Threadpool> threadpool, size_t maxWorkers);

  //! Synchronize and stop the task scheduler.
  //! @param discard @c true Discard the tasks scheduled for the future.
  //! @return A future that signals when the scheduler can be destroyed.
  std::future<void> stop(bool discard = false);

  //! Add a new task to the scheduler.
  //! @param id The unique identity of the task.
  //! @param delay The delay to wait before executing the task.
  //! @param function The function to execute.
  //! @param args the parameters to pass to the function.
  template <typename Duration, class F, class... Args>
  auto scheduleIn(const std::string& id, Duration delay, F&& function, Args&&... args) {
    return this->scheduleAt(id, Detail::Clock::now() + delay, std::forward<F>(function), std::forward<Args>(args)...);
  }

  //! Add a new task to the scheduler.
  //! @param id The unique identity of the task.
  //! @param timepoint The timepoint to reach before executing the task.
  //! @param function The function to execute.
  //! @param args the parameters to pass to the function.
  template <class F, class... Args>
  auto scheduleAt(const std::string& id, Detail::Timepoint timepoint, F&& function, Args&&... args) //
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using ReturnType = typename std::result_of<F(Args...)>::type;
    std::future<ReturnType> future;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>( //
      std::bind(std::forward<F>(function), std::forward<Args>(args)...));
    future = task->get_future();

    auto functor = [this, task = std::move(task)]() {
      (*task)();
      {
        std::lock_guard<std::mutex> guard(_mutex);

        --_workerCount;
        this->processTasks();
      }
    };
    std::lock_guard<std::mutex> guard(_mutex);
    this->addTask(_hasher(id), std::move(functor), timepoint);

    return future;
  }

  //! Add a new periodic task to the scheduler.
  //! @param id The unique identity of the task.
  //! @param delay The minimum duration separating two executions.
  //! @param function The function to execute.
  //! @param args the parameters to pass to the function.
  template <typename Duration, class F, class... Args>
  void scheduleEvery(const std::string& id, Duration delay, F&& function, Args&&... args) {
    auto task = std::bind(std::forward<F>(function), std::forward<Args>(args)...);
    const auto hash = _hasher(id);

    auto periodicTask = [this, hash, task = std::move(task), delay]() mutable {
      task();

      {
        std::lock_guard<std::mutex> guard(_mutex);

        --_workerCount;
        auto periodicTask = _periodicTasks.find(hash);
        if (periodicTask == _periodicTasks.end()) {
          return;
        }
        this->addTask(hash, periodicTask->second, Detail::Clock::now() + delay);
      }
    };
    std::lock_guard<std::mutex> guard(_mutex);
    this->addTask(hash, std::move(periodicTask), Detail::Clock::now() + delay, true);
  }

  //! Remove a task from the scheduler.
  //! @param id The unique identity of the task.
  void remove(const std::string& id);

  //! Check if a task is scheduled.
  //! @param id The unique identity of the task.
  //! @return @c true if it is scheduled, @c false otherwise.
  bool isScheduled(const std::string& id) const;

 private:
  class TimedTask : public Detail::TimedTask {
   public:
    explicit TimedTask(size_t id)
      : Detail::TimedTask(nullptr, {})
      , _id(id) {}
    TimedTask(size_t id, Detail::Task functor, Detail::Timepoint timepoint)
      : Detail::TimedTask(std::move(functor), timepoint)
      , _id(id) {}

    bool operator==(const TimedTask& other) const {
      return _id == other._id;
    }

   private:
    size_t _id;
  };

 private:
  void addTask(size_t hash, std::function<void()> functor, Detail::Timepoint timepoint, bool reschedulable = false);
  void processTasks();

 private:
  std::shared_ptr<Detail::Threadpool> _threadpool;
  Detail::PriorityQueue<TimedTask, std::greater<>> _tasks;
  std::unordered_map<size_t, std::function<void()>> _periodicTasks;
  std::hash<std::string> _hasher;
  mutable std::mutex _mutex;
  size_t _maxWorkers;
  size_t _workerCount{ 0 };
  bool _stopped{ false };
};

} /* namespace Task */

#endif
