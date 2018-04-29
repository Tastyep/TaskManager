#ifndef TASK_DETAIL_PRIORITY_QUEUE_HPP
#define TASK_DETAIL_PRIORITY_QUEUE_HPP

#include <algorithm>
#include <queue>
#include <vector>

namespace Task {
namespace Detail {

template <typename T, typename Comp = std::less<>>
class PriorityQueue : public std::priority_queue<T, std::vector<T>, Comp> {
 public:
  bool erase(const T& e) {
    const auto it = std::find(this->c.begin(), this->c.end(), e);

    if (it == this->c.end()) {
      return false;
    }
    this->c.erase(it);
    std::make_heap(this->c.begin(), this->c.end(), this->comp);

    return true;
  }

  bool update(const T& e) {
    const auto it = std::find(this->c.begin(), this->c.end(), e);

    if (it == this->c.end()) {
      return false;
    }
    *it = e;
    std::make_heap(this->c.begin(), this->c.end(), this->comp);

    return true;
  }

  void clear() {
    this->c.clear();
  }
};

} /* namespace Detail */
} /* namespace Task */

#endif
