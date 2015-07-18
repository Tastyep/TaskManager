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
  std::this_thread::sleep_for(std::chrono::milliseconds(1500));
  tp.stop();
  std::cout << result.get() << std::endl;
}

void test4(ThreadPool& tp) {
  std::future<std::string> result;
  bool stop = false;
  bool pause = false;
  Task task;

  tp.start();
  result = task.assign([&stop, &pause] {
    unsigned int sec = 0;
    unsigned int paused = 0;
    while (not stop) {
      if (not pause) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        sec += 100;
        std::cout << "Active for " << sec << " ms" << std::endl;
      }
      else {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        paused += 100;
        std::cout << "Paused for " << paused << " ms" << std::endl;
      }
    }
    return std::string("Test 4 passed");
  });

  task.setStopFunction([&stop] { stop = true; });
  task.setPauseFunction([&pause] { pause = true; });
  task.setUnpauseFunction([&pause] { pause = false; });

  tp.addTask(task);
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  tp.pause();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  tp.unpause();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
  test4(tp);
  return 0;
}
