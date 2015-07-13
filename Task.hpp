#ifndef TASK_HPP_BASE_
#define TASK_HPP_BASE_

#include <future>
#include <functional>
#include <mutex>

class Task {
public:
  Task() : function(nullptr) {}
  Task(std::nullptr_t nullp) : function(nullptr) {}
  explicit Task(const std::function<void ()>& func) : function(func) {} // no need for future
  Task(const Task& task) :
  function(task.function) {}

  virtual ~Task() = default;

  void operator()() {
    function();
  }

  virtual Task& operator=(const Task& other) {
    this->function = other.function;
    return *this;
  }
  Task& operator=(std::function<void ()> func) {
    this->function = func;
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

  void operator>>(std::function<void ()> newFunc) {
    this->function = [func = this->function, newFunc]() {
      func();
      newFunc();
    };
  }

  void operator<<(std::function<void ()> newFunc) {
    this->function = [func = this->function, newFunc]() {
      newFunc();
      func();
    };
  }

  template<class F, class... Args>
  auto assign(F&& function, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()> >
            (std::bind(std::forward<F>(function), std::forward<Args>(args)...));
    auto future = task->get_future();

    function = [this, task]() {
      (*task)();
    };
    return future;
  }

  virtual void stop() {};

private:
  std::function<void ()> function;
};

#endif /* end of include guard: TASK_HPP_BASE_ */
