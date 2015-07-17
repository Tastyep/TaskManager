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

int main(int argc, char const *argv[]) {
  ThreadManager manager(1);
  ThreadPool tp(1, manager);

  tp.start();
  test1(tp);
  test2(tp);
  return 0;
}
