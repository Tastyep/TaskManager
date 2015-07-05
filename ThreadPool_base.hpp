#ifndef THREADPOOL_BASE_HPP_
#define THREADPOOL_BASE_HPP_

#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <utility>
#include <string>

template<typename T,
template <typename, typename = std::allocator<T>> class Container = std::queue>
class ThreadPool_base {
public:
    enum class state {
      STOP = 0,
      PAUSE,
      START
    };
public:
    ThreadPool_base(unsigned int nbThreads) : nbThreads(nbThreads) {};
    virtual ~ThreadPool_base() {
      if (status.load(std::memory_order_seq_cst) != state::STOP) {
        this->stop();
      }
    };

public:
    std::pair<bool, std::string>
    start() {
      if (status.load(std::memory_order_seq_cst) != state::STOP) {
          return std::make_pair(false, std::string("ThreadPool has already been started"));
      }

      status.store(state::PAUSE, std::memory_order_seq_cst);      // to synchronize threads
      for (unsigned int i = 0; i < this->nbThread; ++i) startThread();
      status.store(state::PLAY, std::memory_order_seq_cst);       // we can now exectue tasks
      return std::make_pair(true, std::string(""));
    };

    std::pair<bool, std::string> pause() {
      if (status.load(std::memory_order_seq_cst) != state::START) {
          return std::make_pair(false, std::string("ThreadPool is not started"));
      }
      status.store(state::PAUSE, std::memory_order_acquire);
      return std::make_pair(true, std::string(""));
    };

    std::pair<bool, std::string> unpause() {
      if (status.load(std::memory_order_seq_cst) != state::PAUSE) {
          return std::make_pair(false, std::string("ThreadPool is not paused"));
      }
      status.store(state::PLAY, std::memory_order_acquire);
      return std::make_pair(true, std::string(""));
    }
    std::pair<bool, std::string>
    stop() {
      if (status.load(std::memory_order_seq_cst) == state::STOP) {
          return std::make_pair(false, std::string("ThreadPool is already stopped"));
      }
      status.store(state::STOP, std::memory_order_acquire);
      return std::make_pair(true, std::string(""));
    };

    void
    startThread() {
      // threads.emplace_back(threadFunction);
    }

    virtual void
    threadFunction() = 0;

protected:
    Container<T>  taskContainer;
    std::mutex    taskMutex;

private:
    std::atomic_bool 	status;
    unsigned int      nbThreads;
};

#endif /* end of include guard: THREADPOOL_BASE_HPP_ */
