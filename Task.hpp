#ifndef TASK_HPP_BASE_
#define TASK_HPP_BASE_

#include <future>
#include <functional>
#include <mutex>
#include <atomic>
#include <vector>

#include <iostream> // debug

class Task {
public:
  Task() :
  function(nullptr),
  stopFunction(nullptr),
  terminated(false) {}

  Task(std::nullptr_t nullp) :
  function(nullptr),
  stopFunction(nullptr),
  terminated(false) {}

  explicit Task(const std::function<void (bool)>& func) :
  function(func),
  stopFunction(nullptr),
  terminated(false) {
  } // no need for future

  Task(const Task& task) :
  function(
  task.function),
  stopFunction(task.stopFunction),
  callbacks(task.callbacks),
  terminated(terminated.load()) {}

  ~Task() = default;

  void operator()() {
    this->function(this->terminated.load());
    std::lock_guard<std::mutex> lock_guard(callbackMutex);
    for (auto callback: this->callbacks) {
      callback();
    }
  }

  Task& operator=(const Task& other) {
    this->function = other.function;
    this->stopFunction = other.stopFunction;
    this->terminated = other.terminated.load();
    this->callbacks = other.callbacks;
    return *this;
  }

  Task& operator=(const std::function<void (bool)> function) {
    this->function = function;
    return *this;
  }

  template<typename T>
  bool operator==(const T& var) {
    return (function == var);
  }

  template<typename T>
  bool operator!=(const T& var) {
    return (function != var);
  }

  template<class F, class... Args>
  auto assign(F&& function, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()> >
            (std::bind(std::forward<F>(function), std::forward<Args>(args)...));
    auto future = task->get_future();

    this->function = [this, task](bool terminated) {
      (*task)(terminated);
    };
    return future;
  }

  template<class F, class... Args>
  void addCallback(F&& function, Args&&... args) {
    auto task = std::bind(std::forward<F>(function), std::forward<Args>(args)...);
    auto wrappedTask = [this, task]() {
      task();
    };

    std::lock_guard<std::mutex> lock_guard(callbackMutex);
    callbacks.emplace_back(wrappedTask);
  }

  template<class F, class... Args>
  void setStopFunction(F&& function, Args&&... args) {
    auto task = std::bind(std::forward<F>(function), std::forward<Args>(args)...);

    this->stopFunction = [this, task]() {
      task();
    };
  }

  const std::function<void ()>& getStopFunction() {
    return this->stopFunction;
  }

  void stop() {
    if (stopFunction)
      stopFunction();
  };

  void abort() {
    terminated.store(true); // add prior
  }

private:
  std::function<void (bool terminated)> function;
  std::function<void ()> stopFunction;

  std::vector<std::function<void ()> > callbacks;
  std::mutex callbackMutex;

  std::atomic_bool terminated;
};

#endif /* end of include guard: TASK_HPP_BASE_ */
