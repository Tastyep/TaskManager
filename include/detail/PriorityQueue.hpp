#ifndef TASK_DETAIL_PRIORITY_QUEUE_HPP
#define TASK_DETAIL_PRIORITY_QUEUE_HPP

#include <algorithm>
#include <queue>
#include <vector>

namespace Task {
namespace Detail {

template <typename T, typename Comp>
class PriorityQueue : public std::priority_queue<T, std::vector<T>, Comp> {
 public:
  template <typename Predicate>
  bool erase(Predicate p) {
    const auto it = std::find_if(this->c.begin(), this->c.end(), p);

    if (it == this->c.end()) {
      return false;
    }
    this->c.erase(it);
    std::make_heap(this->c.begin(), this->c.end(), this->comp);

    return true;
  }

  template <typename Predicate>
  bool update(const T& entity, Predicate p) {
    const auto it = std::find_if(this->c.begin(), this->c.end(), p);

    if (it == this->c.end()) {
      return false;
    }
    *it = entity;
    std::make_heap(this->c.begin(), this->c.end(), this->comp);

    return true;
  }
};

} /* namespace Detail */
} /* namespace Task */

#endif
