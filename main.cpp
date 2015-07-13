#include <chrono>
#include <iostream>
#include "ThreadPool.hh"
#include <unistd.h>
int main(int argc, char const *argv[]) {
  ThreadManager manager(6);
  ThreadPool tp(1, manager);
  std::vector< std::future<unsigned int> > results;

  tp.start();
  for (unsigned int i = 0; i < 8; ++i) {
    results.emplace_back(
      tp.addTask([i] {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        return i;
      })
    );
  }
  for(auto && result: results)
    std::cout << result.get() << ' ';
  std::cout << std::endl;
  return 0;
}
