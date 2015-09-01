#include <chrono>
#include <iostream>
#include "ThreadPool.hh"
#include "Scheduler.hh"
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
  task.setPauseFunction([&pause] { std::cout << "Pass" << std::endl; pause = true; });
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

void test5(ThreadPool& tp) {
  tp.start();
  tp.resize(1);
  for (int i = 0; i < 2; ++i) {
      tp.addTask([i] {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "Task " << i << " finished" << std::endl;
        return i;
      });
  }
}

void schedule1(Scheduler& scheduler) {
  auto future1 = scheduler.runAt([]() {
                  return "Done1";
                }, std::chrono::steady_clock::now() + std::chrono::milliseconds(200));
  auto future2 = scheduler.runIn([]() {
                  return "Done2";
                }, std::chrono::milliseconds(200));

  std::cout << future1.get() << std::endl;
  std::cout << future2.get() << std::endl;
}

void schedule2(Scheduler& scheduler) {
  std::vector<std::future<int>> futures;

  for (int i = 0; i < 10; ++i) {
    futures.emplace_back(scheduler.runIn([i]() {
                          return i;
                        }, std::chrono::milliseconds(100 * i))
                      );
  }
  for (auto& future : futures)
    std::cout << future.get() << std::endl;
}

void schedule3(Scheduler& scheduler) {
  scheduler.runEvery([]() {
                        std::cout << "Every second" << std::endl;
                    }, std::chrono::milliseconds(1000));
  scheduler.runIn([]() {
      std::cout << "3 seconds elapsed" << std::endl;
  }, std::chrono::seconds(3));
  std::this_thread::sleep_for(std::chrono::seconds(5));
}

void schedule4(ThreadManager& manager) {
  Scheduler scheduler(2, manager);
  bool stop = false;

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  Task task([&stop, &scheduler]() {
    unsigned int i = 1;
    while (!stop) {
      auto future = scheduler.runIn([i]() {
        return "";//std::to_string(i) + " second elapsed";
      }, std::chrono::milliseconds(60));
      std::cout << future.get() << std::endl;
      ++i;
    }
  });
  task.setStopFunction([&stop]() {
    stop = true;
  });
  std::cout << "before runAt" << std::endl;
  scheduler.runAt(task, std::chrono::steady_clock::now());
  //std::cout << "Press a key to continue" << std::endl;
  getchar();
  std::cout << "end" << std::endl;
}

int main(int argc, char const *argv[]) {
  ThreadManager manager(1);
//  ThreadPool tp(2, manager);
  Scheduler scheduler(2, manager);

  // tp.start();
  // test1(tp);
  // test2(tp);
  // test3(tp);
  // test4(tp);
  // test5(tp);

//  std::cout << "------- Scheduler -------" << std::endl;

  // schedule1(scheduler);
  // schedule2(scheduler);
//  schedule3(scheduler);
  schedule4(manager);
  return 0;
}
