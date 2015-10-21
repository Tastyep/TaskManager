#include "Worker.hh"

#include <iostream> //debug

namespace TaskManager {

Worker::Worker() : thread(), task(nullptr), running(false), reserved(false) {}

Worker::~Worker() {
    if (this->running.load()) this->stop();
    this->waitStopped();
};

void
Worker::start(std::condition_variable& cv, std::mutex& condvarMutex) {
    if (this->running.load() == true)
        throw std::runtime_error("Can't start an already running worker");
    this->running.store(true);
    this->thread = std::thread(&Worker::threadMain, this, std::ref(cv), std::ref(condvarMutex));
}

void
Worker::stop() {
    this->running.store(false);
}

void
Worker::stopTask() {
    std::lock_guard<std::mutex> guard(this->taskMutex);
    if (this->task != nullptr) { this->task.stop(); }
}

void
Worker::pauseTask() {
    std::lock_guard<std::mutex> guard(this->taskMutex);
    if (this->task != nullptr) { this->task.pause(); }
}

void
Worker::unpauseTask() {
    std::lock_guard<std::mutex> guard(this->taskMutex);
    if (this->task != nullptr) { this->task.unpause(); }
}

void
Worker::waitStopped() {
    if (this->thread.joinable()) this->thread.join();
}

void
Worker::threadMain(std::condition_variable& cv, std::mutex& condvarMutex) {
    while (this->running.load()) {
        {
            std::unique_lock<std::mutex> lock(condvarMutex);

            cv.wait(lock,
                    [this] {
                        std::lock_guard<std::mutex> guard(this->taskMutex);
                        return (not this->running.load() || this->task != nullptr);
                    });
            if (not this->running.load()) return;
        }
        this->task();
        this->setReserved(false);
        this->setTask(Task(nullptr));
    }
}

void
Worker::setTask(const std::function<void()>& task) {
    std::lock_guard<std::mutex> taskGuard(this->taskMutex);
    this->task = task;
}

void
Worker::setTask(const Task& task) {
    std::lock_guard<std::mutex> taskGuard(this->taskMutex);
    this->task = task;
}

bool
Worker::isIdle() {
    std::lock_guard<std::mutex> guard(this->taskMutex);
    return (this->task == nullptr);
}

void
Worker::setReserved(bool status) {
    this->reserved.store(status);
}

bool
Worker::isReserved() {
    return this->reserved.load();
}
}
