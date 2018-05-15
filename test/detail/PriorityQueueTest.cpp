#include "test/detail/PriorityQueueTest.hpp"

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

TEST_F(DetailPriorityQueue, EraseInexistant) {
  PriorityQueue<int> queue;

  EXPECT_FALSE(queue.erase(1));
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

TEST_F(DetailPriorityQueue, UpdateInexistant) {
  PriorityQueue<Entity> queue;

  EXPECT_FALSE(queue.update(Entity{ 1, 1 }));
}

TEST_F(DetailPriorityQueue, Contain) {
  PriorityQueue<Entity> queue;

  queue.push(Entity{ 4, 1 });
  EXPECT_TRUE(queue.contain(Entity{ 4, 0 }));
}

TEST_F(DetailPriorityQueue, ContainNotFound) {
  PriorityQueue<Entity> queue;

  EXPECT_FALSE(queue.contain(Entity{ 4, 0 }));
}

TEST_F(DetailPriorityQueue, Clear) {
  PriorityQueue<int> queue;

  for (auto i : { 2, 3, 4, 5 }) {
    queue.push(i);
  }

  queue.clear();
  EXPECT_THAT(this->makeVector(queue), IsEmpty());
}

} /* namespace Detail */
} /* namespace Task */
