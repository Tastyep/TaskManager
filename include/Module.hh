#ifndef TASK_MODULE_HH
#define TASK_MODULE_HH

#include <cstddef>
#include <memory>

namespace Task {

class Manager;
class Scheduler;

namespace Module {

void init(size_t workerCount);
std::shared_ptr<Manager> makeManager(size_t workerCount);
std::shared_ptr<Scheduler> makeScheduler(size_t workerCount);

} /* namespace Module */
} /* namespace Task */

#endif
