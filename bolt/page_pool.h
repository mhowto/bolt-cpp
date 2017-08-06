#ifndef __BOLT_PAGE_POOL_H
#define __BOLT_PAGE_POOL_H

#include "slice.h"
#include <vector>

class PagePool {
public:
  PagePool(std::function<Slice()> new_page) : new_page_(new_page) {}
  Slice get();
  void put(const Slice &);

private:
  std::vector<Slice> pages_;
  std::function<Slice()> new_page_;
};

#endif