#include "test/detail/PriorityQueueTest.hh"

namespace Task {
namespace Detail {

TEST_F(DetailPriorityQueue, Erase) {
  PriorityQueue<int> queue;

  for (auto i : { 2, 3, 4, 5 }) {
    queue.push(i);
  }

  queue.erase(4);
  queue.erase(2);
  EXPECT_THAT(this->makeVector(queue), ElementsAreArray({ 5, 3 }));
}

} /* namespace Detail */
} /* namespace Task */
