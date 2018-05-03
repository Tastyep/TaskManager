#include "Module.hh"

#include <cassert>

#include "Manager.hh"
#include "Scheduler.hh"
#include "detail/Threadpool.hh"

namespace Task {
namespace Module {
namespace {

std::shared_ptr<Detail::Threadpool> _threadpool;

} /* namespace */

void init(size_t workerCount) {
  assert(!_threadpool);
  _threadpool = std::make_shared<Detail::Threadpool>(workerCount);
}

std::shared_ptr<Manager> makeManager(size_t workerCount) {
  assert(_threadpool);
  return std::make_shared<Manager>(_threadpool, workerCount);
}

std::shared_ptr<Scheduler> makeScheduler(size_t workerCount) {
  assert(_threadpool);
  return std::make_shared<Scheduler>(_threadpool, workerCount);
}

} /* namespace Module */
} /* namespace Task */
