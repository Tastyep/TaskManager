#include "Worker.hh"

#include <iostream> //debug

Worker::Worker() :
thread()
, task(nullptr)
, running(false) {
}

Worker::~Worker() {
  if (this->running)
    this->stop();
  this->waitStopped();
};

void
Worker::start(std::condition_variable& cv,
      std::mutex& condvarMutex) {
  this->running = true;
  this->thread = std::thread(&Worker::threadMain, this, std::ref(cv), std::ref(condvarMutex));
}

void
Worker::stop() {
  this->running = false;
}

void
Worker::stopTask() {
  std::lock_guard<std::mutex> guard(this->taskMutex);
  if (this->task != nullptr) {
    this->task.stop();
  }
}

void
Worker::pauseTask() {
  std::lock_guard<std::mutex> guard(this->taskMutex);
  if (this->task != nullptr) {
    this->task.pause();
  }
}

void
Worker::unpauseTask() {
  std::lock_guard<std::mutex> guard(this->taskMutex);
  if (this->task != nullptr) {
    this->task.unpause();
  }
}

void
Worker::waitStopped() {
  if (this->thread.joinable())
    this->thread.join();
}

void
Worker::threadMain(std::condition_variable& cv, std::mutex& condvarMutex) {
  while (this->running) {
    {
      std::unique_lock<std::mutex> lock(condvarMutex);

      cv.wait(lock, [this] {
        std::lock_guard<std::mutex> guard(this->mutex);
        return (not this->running || this->task != nullptr);
      });
      if (not this->running)
        return ;
    }
    this->task();
    this->setTask(Task(nullptr));
  }
}

Task&
Worker::getTask() {
  std::lock_guard<std::mutex> guard(this->mutex);
  return this->task;
}

void
Worker::setTask(const std::function<void ()>& task) {
  std::lock_guard<std::mutex> guard(this->mutex);
  std::lock_guard<std::mutex> taskGuard(this->taskMutex);
  this->task = task;
}

void
Worker::setTask(const Task& task) {
  std::lock_guard<std::mutex> guard(this->mutex);
  std::lock_guard<std::mutex> taskGuard(this->taskMutex);
  this->task = task;
}

bool
Worker::isIdle() {
  std::lock_guard<std::mutex> guard(this->mutex);
  return (this->task == nullptr);
}
