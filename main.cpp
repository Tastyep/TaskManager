#include <chrono>
#include <iostream>
#include "ThreadPool.hh"
#include <unistd.h>

void test1(ThreadPool& tp) {
  std::vector< std::future<int> > results;

  for (int i = 0; i < 8; ++i) {
    results.emplace_back(
      tp.addTask([i] {
        return i;
      })
    );
  }
  for(auto && result: results)
    std::cout << result.get() << ' ';
  std::cout << std::endl;
}

void test2(ThreadPool& tp) {
  std::vector< std::future<int> > results;

  for (int i = 0; i < 4; ++i) {
    results.emplace_back(
      tp.addTask([i] {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        return i;
      })
    );
  }
  tp.stop();
  for(auto && result: results)
    std::cout << result.get() << ' ';
  std::cout << std::endl;
}

void test3(ThreadPool& tp) {
  std::future<std::string> result;
  bool stop = false;
  Task task;

  tp.start();
  result = task.assign([&stop] {
    unsigned int sec = 0;
    while (not stop) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      sec += 100;
      std::cout << "Active for " << sec << " ms" << std::endl;
    }
    return std::string("Test 3 passed");
  });
  task.setStopFunction([&stop] { stop = true; });
  tp.addTask(task);
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  tp.stop();
  std::cout << result.get() << std::endl;
}

int main(int argc, char const *argv[]) {
  ThreadManager manager(1);
  ThreadPool tp(1, manager);

  tp.start();
  test1(tp);
  test2(tp);
  test3(tp);
  return 0;
}
