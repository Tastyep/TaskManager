#include "Scheduler.hh"

Scheduler::Scheduler(unsigned int nbThreads,
                     ThreadManager& manager) : status(state::START)
{}

Scheduler::~Scheduler() {
  if (this->status.load(std::memory_order_seq_cst) != state::STOP) {
    // this->stop();
  }
}

void
Scheduler::runAt(const Task& task,
                 const std::chrono::steady_clock::time_point& timePoint) {
  if (this->status.load(std::memory_order_release) == state::STOP)
    throw std::runtime_error("Can't add task on stopped Scheduler");
  std::lock_guard<std::mutex> guard(this->taskMutex);
  this->taskContainer.emplace_back(task, timePoint);
}

void
Scheduler::runEvery(const Task& task,
                    const std::chrono::steady_clock::duration& duration) {
  if (this->status.load(std::memory_order_release) == state::STOP)
    throw std::runtime_error("Can't add task on stopped Scheduler");

  auto cuNow = std::chrono::steady_clock::now() + duration;
  this->runAt(Task([this, task(task), duration]() mutable {
      task();
      auto now = std::chrono::steady_clock::now();
      this->runAt(task, now + duration);
    }), cuNow);
}
