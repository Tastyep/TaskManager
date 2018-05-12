#ifndef TASK_SCHEDULER_HH
#define TASK_SCHEDULER_HH

#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "detail/PriorityQueue.hpp"
#include "detail/Task.hpp"
#include "detail/Threadpool.hh"

namespace Task {

class Scheduler {
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

 public:
  Scheduler(std::shared_ptr<Detail::Threadpool> threadpool, size_t maxWorkers);

  std::future<void> stop(bool discard = false);

  template <typename Duration, class F, class... Args>
  auto scheduleIn(const std::string& id, Duration delay, F&& function, Args&&... args) {
    return this->scheduleAt(id, Detail::Clock::now() + delay, std::forward<F>(function), std::forward<Args>(args)...);
  }

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
    this->addTask(id, std::move(functor), timepoint);

    return future;
  }

  template <typename Duration, class F, class... Args>
  void scheduleEvery(const std::string& id, Duration delay, F&& function, Args&&... args) {
    auto task = std::bind(std::forward<F>(function), std::forward<Args>(args)...);

    auto periodicTask = [this, id, task = std::move(task), delay]() {
      task();

      {
        std::lock_guard<std::mutex> guard(_mutex);

        --_workerCount;
        auto periodicTask = _periodicTasks.find(id);
        if (periodicTask == _periodicTasks.end()) {
          return;
        }
        this->addTask(id, periodicTask->second, Detail::Clock::now() + delay);
      }
    };
    std::lock_guard<std::mutex> guard(_mutex);
    this->addTask(id, std::move(periodicTask), Detail::Clock::now() + delay, true);
  }

  void remove(const std::string& id);
  bool isScheduled(const std::string& id) const;

 private:
  void addTask(const std::string& id, std::function<void()> functor, Detail::Timepoint timepoint,
               bool reschedulable = false);
  void processTasks();

 private:
  std::shared_ptr<Detail::Threadpool> _threadpool;
  Detail::PriorityQueue<TimedTask, std::greater<>> _tasks;
  std::unordered_map<std::string, std::function<void()>> _periodicTasks;
  std::hash<std::string> _hasher;
  mutable std::mutex _mutex;
  size_t _maxWorkers;
  size_t _workerCount{ 0 };
  bool _stopped{ false };
};

} /* namespace Task */

#endif
