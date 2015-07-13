#ifndef THREADPOOL_HH_
#define THREADPOOL_HH_

#include <queue>
#include <stdexcept>

#include "ThreadManager.hh"

class ThreadPool {
public:
    enum class state {
      STOP = 0,
      PAUSE,
      START
    };
public:
    ThreadPool(unsigned int nbThreads,
                    ThreadManager& manager);

    virtual ~ThreadPool();

public:
  std::pair<bool, std::string> start();
  std::pair<bool, std::string> pause();
  std::pair<bool, std::string> unpause();
  std::pair<bool, std::string> stop();

  void addTask(Task& task);

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
      taskContainer.emplace(Task([this, task]() {
        (*task)();
        {
          std::lock_guard<std::mutex> guardRef(this->refCountMutex);
          --(this->threadRefCount);
        }
        this->startTask();
      }));
    }
    this->startTask();
    return futureResult;
  }

private:
  void
  startTask() {
    std::lock_guard<std::mutex> guardRef(this->refCountMutex);
    std::lock_guard<std::mutex> guardTask(this->taskMutex);
    Task task;

    if (this->taskContainer.empty()
    || this->threadRefCount >= this->maxParallelism
    || this->status.load(std::memory_order_release) != state::START) // Handle later or already handled
      return ;
    ++this->threadRefCount;
    task = std::move(this->taskContainer.front());
    this->taskContainer.pop();
    manager.runTask(task);
  }

private:
    std::queue<Task> taskContainer;
    std::mutex       taskMutex;

    unsigned int  threadRefCount;
    std::mutex    refCountMutex;

    ThreadManager& manager;

    std::atomic<state> 	status;
    unsigned int      maxParallelism;
};

#endif /* end of include guard: THREADPOOL_HH_ */
