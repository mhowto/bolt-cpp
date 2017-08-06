#include "page_pool.h"

Slice PagePool::get() {
  if (this->pages_.empty()) {
    return new_page_();
  }
  return this->pages_.pop_back();
}

void PagePool::put(Slice s) { this->pages_.push_back(s); }