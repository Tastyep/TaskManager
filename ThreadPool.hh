#ifndef THREADPOOL_HH_
#define THREADPOOL_HH_

#include <queue>
#include <stdexcept>

#include "ThreadManager.hh"

class ThreadPool {
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
  auto addTask(typename std::result_of<F(Args...)>::type defValue,
  F&& function, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    std::future<return_type> futureResult;

    if (this->status.load(std::memory_order_release) == state::STOP)
      throw std::runtime_error("Can't add task on stopped ThreadPool");

    auto task = std::bind(std::forward<F>(function), std::forward<Args>(args)...);
    auto taskWrapper = [defValue, task](bool terminated) {
      if (terminated)
        return defValue;
      return task;
    };
    auto packagedTask = std::make_shared<std::packaged_task<return_type()> >(taskWrapper);
    futureResult = task->get_future();

    {
      std::lock_guard<std::mutex> guard(this->taskMutex);
      this->taskContainer.emplace([this, packagedTask]() {
        (*packagedTask)();
      });
      // {
      //   std::lock_guard<std::mutex> guardRef(this->refCountMutex);
      //   --(this->threadRefCount);
      // }
      // this->startTask();
    }
    this->startTask();
    return futureResult;
  }

private:
  void startTask();
  void decreaseRefCount();
  void removeWorkerRef(std::shared_ptr<Worker> worker);
  void emptyTasks();

private:
    std::queue<Task> taskContainer;
    std::mutex       taskMutex;

    unsigned int  threadRefCount;
    std::mutex    refCountMutex;

    ThreadManager& manager;

    std::atomic<state> 	status;
    unsigned int        maxParallelism;

    std::vector<std::shared_ptr<Worker> > workers;
    std::mutex workerMutex;
};

#endif /* end of include guard: THREADPOOL_HH_ */
