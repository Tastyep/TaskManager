#ifndef TASK_HPP_
#define TASK_HPP_

#include <future>
#include <functional>
#include <mutex>

class Task {
public:
  Task() : function(nullptr) {}
  Task(std::nullptr_t nullp) : function(nullptr) {}
  explicit Task(const std::function<void ()>& func) : function(func) {} // no need for future
  Task(const Task& task) :
  function(task.function)
  , startCallbacks(task.startCallbacks)
  , endCallbacks(task.endCallbacks)
  , callbackMutex() {
  }
  ~Task() = default;


  void operator()() {
    function();
  }

  Task& operator=(const Task& other) {
    this->function = other.function;
    this->startCallbacks = other.startCallbacks;
    this->endCallbacks = other.endCallbacks;
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

    function = [this, task]() {
      {
        std::lock_guard<std::mutex> guard(this->callbackMutex);
        for (auto& callback : startCallbacks) {
          callback();
        }
      }
      (*task)();
      {
        std::lock_guard<std::mutex> guard(this->callbackMutex);
        for (auto& callback : endCallbacks) {
          callback();
        }
      }
    };
    return future;
  }

  void addCallbackBefore(const std::function<void ()>& func) {
    startCallbacks.push_back(func);
  }

  void addCallbackAfter(const std::function<void ()>& func) {
    endCallbacks.push_back(func);
  }


private:
  std::function<void ()> function;
  std::vector<std::function<void ()>> startCallbacks;
  std::vector<std::function<void ()>> endCallbacks;
  std::mutex  callbackMutex;
};

#endif /* end of include guard: TASK_HPP_ */
