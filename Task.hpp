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
  stopFunction(nullptr) {}

  Task(std::nullptr_t nullp) :
  function(nullptr),
  stopFunction(nullptr) {}

  explicit Task(const std::function<void ()>& func) :
  function(func),
  stopFunction(nullptr) {} // no need for future

  Task(const Task& task) :
  function(
  task.function),
  stopFunction(task.stopFunction),
  callbacks(task.callbacks) {}

  ~Task() = default;

  void operator()() {
    this->function();
    std::lock_guard<std::mutex> lock_guard(callbackMutex);
    for (auto callback: this->callbacks) {
      callback();
    }
  }

  Task& operator=(const Task& other) {
    this->function = other.function;
    this->stopFunction = other.stopFunction;
    this->callbacks = other.callbacks;
    return *this;
  }

  Task& operator=(const std::function<void ()> function) {
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

    this->function = [this, task]() {
      (*task)();
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

  template<class F, class... Args>
  void setPauseFunction(F&& function, Args&&... args) {
    auto task = std::bind(std::forward<F>(function), std::forward<Args>(args)...);

    this->pauseFunction = [this, task]() {
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

  void pause() {
    if (pauseFunction)
      pauseFunction();
  };

private:
  std::function<void ()> function;
  std::function<void ()> stopFunction;
  std::function<void ()> pauseFunction;

  std::vector<std::function<void ()> > callbacks;
  std::mutex callbackMutex;
};

#endif /* end of include guard: TASK_HPP_BASE_ */
