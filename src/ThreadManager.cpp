#include "ThreadManager.hh"
#include <iostream>
#include <chrono>

namespace TaskManager {

ThreadManager::ThreadManager(unsigned int nbThread) : running(true) {
    for (unsigned int i = 0; i < nbThread; ++i) { this->addNewThread(); }
}

ThreadManager::~ThreadManager() {
    if (this->running.load()) this->stop();
    {
        std::lock_guard<std::mutex> guard(this->workersMutex);
        workers.clear();
    }
}

void
ThreadManager::addNewThread() {
    std::lock_guard<std::mutex> guard(this->workersMutex);

    this->workers.emplace_back(new Worker);
    this->workers.back()->start(this->cv, this->condvarMutex);
}

unsigned int
ThreadManager::roundToNextPower(unsigned int nbThread) const {
    nbThread |= nbThread >> 1;
    nbThread |= nbThread >> 2;
    nbThread |= nbThread >> 4;
    nbThread |= nbThread >> 8;
    nbThread |= nbThread >> 16;
    ++nbThread;
    return nbThread;
}

std::shared_ptr<Worker>
ThreadManager::getWorker() {
    unsigned int maxThreads;
    unsigned int nbWorkers;

    if (not this->running.load()) return nullptr;
    {
        std::lock_guard<std::mutex> guard(this->workersMutex);

        for (auto& worker : this->workers) {
            if (worker->isIdle() && not worker->isReserved()) {
                worker->setReserved(true);
                return worker;
            }
        }
    }
    {
        std::lock_guard<std::mutex> guard(this->workersMutex);
        nbWorkers = this->workers.size();
        maxThreads = roundToNextPower(nbWorkers);
    }
    for (unsigned int i = 0; i < maxThreads - nbWorkers; ++i) { this->addNewThread(); }
    return this->getWorker();
}

void
ThreadManager::startTask(std::shared_ptr<Worker> worker, const Task& task) {
    worker->setTask(task);
    this->cv.notify_all();
}

std::pair<bool, std::string>
ThreadManager::stop() {
    std::lock_guard<std::mutex> guard(this->workersMutex);

    if (this->running.load() == false) {
        return std::make_pair(false, "ThreadManager is already stopped");
    }
    this->running.store(false);
    for (auto& worker : this->workers) worker->stop();
    this->cv.notify_all();
    return std::make_pair(true, "");
};
}
