#ifndef __BOLT_PAGE_POOL_H
#define __BOLT_PAGE_POOL_H

#include "slice.h"
#include <functional>
#include <mutex>
#include <vector>

class PagePool {
public:
  PagePool(std::function<Slice()> new_page) : new_page_(new_page) {}

  // once you get Slice from page pool, you should charge the slice's life.
  Slice get();
  void put(const Slice &);

private:
  std::mutex _pool_mutex; // protects page pool
  std::vector<Slice> pages_;
  std::function<Slice()> new_page_;
};

#endif