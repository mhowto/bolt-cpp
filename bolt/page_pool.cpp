#include "page_pool.h"

Slice PagePool::get() {
  std::lock_guard<std::mutex> lock(_pool_mutex);
  if (this->pages_.empty()) {
    return new_page_();
  }
  Slice &s = this->pages_.back();
  this->pages_.pop_back();
  return s;
}

void PagePool::put(const Slice &s) {
  std::lock_guard<std::mutex> lock(_pool_mutex);
  this->pages_.push_back(s);
}