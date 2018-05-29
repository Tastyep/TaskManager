#ifndef TASK_MANAGER_TEST_DETAIL_PRIORITY_QUEUE_HPP
#define TASK_MANAGER_TEST_DETAIL_PRIORITY_QUEUE_HPP

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "TaskManager/detail/PriorityQueue.hpp"

using testing::ElementsAreArray;
using testing::IsEmpty;
using testing::Test;

namespace Task {
namespace Detail {

class DetailPriorityQueue : public Test {
 public:
  DetailPriorityQueue() = default;
  ~DetailPriorityQueue() = default;

  template <typename T>
  std::vector<T> makeVector(PriorityQueue<T> queue) const {
    std::vector<T> vec;

    while (!queue.empty()) {
      vec.push_back(queue.top());
      queue.pop();
    }

    return vec;
  }

 protected:
  struct Entity {
    int id;
    int value;

    bool operator==(const Entity& other) const {
      return id == other.id;
    }
    bool operator<(const Entity& other) const {
      return value < other.value;
    }
  };
};

} /* namespace Detail */
} /* namespace Task */

#endif
