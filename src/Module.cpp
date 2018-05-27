#include "TaskManager/Module.hpp"

#include <cassert>

#include "TaskManager/Manager.hpp"
#include "TaskManager/Scheduler.hpp"
#include "TaskManager/detail/Threadpool.hpp"

namespace Task {
namespace Module {
namespace {

std::shared_ptr<Detail::Threadpool> _threadpool;

} /* namespace */

void init(size_t workerCount) {
  assert(!_threadpool);
  _threadpool = std::make_shared<Detail::Threadpool>(workerCount);
}

std::unique_ptr<Manager> makeManager(size_t workerCount) {
  assert(_threadpool);
  return std::make_unique<Manager>(_threadpool, workerCount);
}

std::unique_ptr<Scheduler> makeScheduler(size_t workerCount) {
  assert(_threadpool);
  return std::make_unique<Scheduler>(_threadpool, workerCount);
}

} /* namespace Module */
} /* namespace Task */
