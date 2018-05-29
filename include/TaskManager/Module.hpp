#ifndef TASK_MANAGER_MODULE_HPP
#define TASK_MANAGER_MODULE_HPP

#include <cstddef>
#include <memory>

namespace Task {

// Forward declarations
class Manager;
class Scheduler;

namespace Module {

//! Initialize the task module.
//! It creates the threadpool instance used by the managers and schedulers.
//! @param workerCount The number of workers managed in the pool.
void init(size_t workerCount);

//! Factory function that creates a new task manager.
//! @param workerCount The maximum number of parallel executions.
//! Setting one worker will make the manager serialize the tasks execution (cf strand from boost asio).
//! @return A new task manager.
std::unique_ptr<Manager> makeManager(size_t workerCount);

//! Factory function that creates a new task scheduler.
//! @param workerCount The maximum number of parallel executions.
//! @return A new task scheduler.
std::unique_ptr<Scheduler> makeScheduler(size_t workerCount);

} /* namespace Module */
} /* namespace Task */

#endif
