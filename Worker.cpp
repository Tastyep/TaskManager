#include "Worker.hh"

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
  bool active;
  this->running = false;
  {
    std::lock_guard<std::mutex> guard(this->mutex);
    active = this->task != nullptr;
  }
  if (active)
  this->task.stop();
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
    this->setTask(nullptr);
  }
}

Task&
Worker::getTask() {
  return this->task;
}

void
Worker::setTask(const std::function<void ()>& task) {
  std::lock_guard<std::mutex> guard(this->mutex);
  this->task = task;
}

bool
Worker::isIdle() {
  std::lock_guard<std::mutex> guard(this->mutex);
  return (this->task == nullptr);
}
