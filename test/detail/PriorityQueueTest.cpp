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

TEST_F(DetailPriorityQueue, Update) {
  PriorityQueue<Entity> queue;

  for (auto i : { 5, 4, 3, 2 }) {
    queue.push(Entity{ i, i });
  }

  queue.update(Entity{ 4, 1 });
  queue.update(Entity{ 2, 6 });
  EXPECT_THAT(this->makeVector(queue), ElementsAreArray({
                                         Entity{ 2, 6 },
                                         Entity{ 5, 5 },
                                         Entity{ 3, 3 },
                                         Entity{ 4, 1 },
                                       }));
}

} /* namespace Detail */
} /* namespace Task */
