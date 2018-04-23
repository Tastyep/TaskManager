#ifndef TASK_TEST_DETAIL_PRIORITY_QUEUE_HH
#define TASK_TEST_DETAIL_PRIORITY_QUEUE_HH

#include "gtest/gtest.h"

#include "detail/PriorityQueue.hpp"

namespace Task {
namespace Detail {

using testing::Test;

class DetailPriorityQueue : public Test {
 public:
  DetailPriorityQueue() = default;
  ~DetailPriorityQueue() = default;
};

} /* namespace Detail */
} /* namespace Task */

#endif
