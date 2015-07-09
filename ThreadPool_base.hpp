#ifndef THREADPOOL_BASE_HPP_
#define THREADPOOL_BASE_HPP_

#include <queue>
#include <stdexcept>
#include <future>

#include "ThreadManager.hh"

template<class T>
class ThreadPool_base {
public:
    enum class state {
      STOP = 0,
      PAUSE,
      START
    };
public:
    ThreadPool_base(unsigned int nbThreads,
                    ThreadManager& manager) :
  maxParallelism(nbThreads)
  , manager(manager)
  , status(state::STOP) {
      if (nbThreads == 0)
        throw std::out_of_range("The ThreadPool must have at least a thread");
    };

    virtual ~ThreadPool_base() {
      if (this->status.load(std::memory_order_seq_cst) != state::STOP) {
        this->stop();
      }
    };

public:
    std::pair<bool, std::string>
    start() {
      if (this->status.load(std::memory_order_seq_cst) != state::STOP) {
          return std::make_pair(false, "ThreadPool has already been started");
      }

<<<<<<< HEAD
      this->status.store(state::PAUSE, std::memory_order_seq_cst);      // to synchronize threads
      for (unsigned int i = 0; i < this->nbThread; ++i)
        this->startTask();
      this->status.store(state::PLAY, std::memory_order_seq_cst);       // we can now exectue tasks
=======
      status.store(state::START, std::memory_order_seq_cst);       // we can now exectue tasks
>>>>>>> 2de02cbf7fb33a0f7a8881edc5534fbb3fb33f2a
      return std::make_pair(true, "");
    };

    std::pair<bool, std::string> pause() {
      if (this->status.load(std::memory_order_seq_cst) != state::START) {
          return std::make_pair(false, "ThreadPool is not started");
      }
      this->status.store(state::PAUSE, std::memory_order_acquire);
      return std::make_pair(true, "");
    };

    std::pair<bool, std::string> unpause() {
      if (this->status.load(std::memory_order_seq_cst) != state::PAUSE) {
          return std::make_pair(false, "ThreadPool is not paused");
      }
<<<<<<< HEAD
      this->status.store(state::PLAY, std::memory_order_acquire);
=======
      status.store(state::START, std::memory_order_acquire);
>>>>>>> 2de02cbf7fb33a0f7a8881edc5534fbb3fb33f2a
      return std::make_pair(true, "");
    }
    std::pair<bool, std::string>
    stop() {
      if (this->status.load(std::memory_order_seq_cst) == state::STOP) {
          return std::make_pair(false, "ThreadPool is already stopped");
      }
      this->status.store(state::STOP, std::memory_order_acquire);
      return std::make_pair(true, "");
    };

public:
  template<class F, class... Args>
  auto addTask(F&& function, Args&&... args)
      -> std::future<typename std::result_of<F(Args...)>::type> {
      using return_type = typename std::result_of<F(Args...)>::type;

      if (this->status.load(std::memory_order_release) == state::STOP)
        throw std::runtime_error("Can't add task on stopped ThreadPool");

      auto task = std::make_shared<std::packaged_task<return_type()> >
              (std::bind(std::forward<F>(function), std::forward<Args>(args)...));
      std::future<return_type> futureResult = task->get_future();

      {
        std::lock_guard<std::mutex> guard(this->taskMutex);
        taskContainer.emplace([this, task]() {
          (*task)();
          {
            std::lock_guard<std::mutex> guardRef(this->refCountMutex);
            --(this->threadRefCount);
          }
          this->startTask();
        });
      }
    bool startCondition;
    {
      std::lock_guard<std::mutex> guardRef(this->refCountMutex);
      startCondition = this->threadRefCount < this->maxParallelism;
    }
    if (startCondition) {
      this->startTask();
    }
    return futureResult;
  }

private:
  void
  startTask() {
    std::lock_guard<std::mutex> guardRef(this->refCountMutex);
    std::lock_guard<std::mutex> guardTask(this->taskMutex);
    std::function<void ()> task;

    if (this->taskContainer.empty()
    || this->threadRefCount >= this->maxParallelism) // Handle later or already handled
      return ;
    task = std::move(this->taskContainer.front());
    this->taskContainer.pop();
  }

protected:
    std::queue<T> taskContainer;
    std::mutex    taskMutex;

    unsigned int  threadRefCount;
    std::mutex    refCountMutex;

    ThreadManager& manager;

private:
    std::atomic<state> 	status;
    unsigned int      maxParallelism;
};

#endif /* end of include guard: THREADPOOL_BASE_HPP_ */
