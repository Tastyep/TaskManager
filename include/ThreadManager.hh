#ifndef THREADMANAGER_HH_
#define THREADMANAGER_HH_

#include <atomic>
#include <thread>
#include <atomic>
#include <memory>
#include <condition_variable>
#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include "Worker.hh"

namespace TaskManager {

class ThreadManager {
public:
    ThreadManager(unsigned int nbThread = std::thread::hardware_concurrency());
    ~ThreadManager();

    std::pair<bool, std::string> stop();
    std::shared_ptr<Worker> getWorker();
    void startTask(std::shared_ptr<Worker> worker, const Task& task);

private:
    unsigned int roundToNextPower(unsigned int nbThread) const;
    void addNewThread();

private:
    std::vector<std::shared_ptr<Worker>> workers;
    std::mutex workersMutex;

    mutable std::condition_variable cv;
    mutable std::mutex condvarMutex;

    std::atomic_bool running;
};
}

#endif /* end of include guard: THREADMANAGER_HH_ */
