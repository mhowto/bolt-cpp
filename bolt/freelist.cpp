#include "freelist.h"
#include "page.h"
#include <algorithm>

void FreeList::release(txid_t txid) {
  std::vector<txid_t> m;
  for (auto it = this->pending.begin(); it != this->pending.end();) {
    if (it->first <= txid) {
      // move transaction's pending pages to the available freelists.
      // Don't remove from the cache since the page is still free.
      m.insert(m.end(), it->second.begin(), it->second.end());
      this->pending.erase(it++);
    } else {
      ++it;
    }
  }
  auto raw_size = this->ids.size();
  this->ids.insert(this->ids.end(), m.begin(), m.end());
  std::inplace_merge(this->ids.begin(), this->ids.begin() + raw_size, this->ids.end());
}

void FreeList::read(Page *p) {
  // If the page.count is at the max uint16 value(64k) then it's considered
  // an overflow and the size of the freelist is storeed as the first element.
  int idx = 0;
  std::uint64_t count = static_cast<std::uint64_t>(p->count());
  if (count == 0xFFFF) {
    idx = 1;
    count = static_cast<std::uint64_t>((reinterpret_cast<pgid_t *>(p->ptr()))[0]);
  }

  // Copy the list of page ids from the freelist.
  if (count > 0) {
    pgid_t *ids = reinterpret_cast<pgid_t *>(p->ptr());
    this->ids.insert(this->ids.end(), ids + idx, ids + count);

    // Make sure they're sorted.
    std::sort(this->ids.begin(), this->ids.end());
  }

  // Rebuild the page cache.
  this->reindex();
}

void FreeList::reindex() {
  this->cache.clear();
  for (auto &it : this->ids) {
    this->cache.insert(it);
  }
  for (auto pair : this->pending) {
    for (auto &it : pair.second) {
      this->cache.insert(it);
    }
  }
}

void FreeList::rollback(txid_t txid) {
}

void FreeList::reload(Page *p) {
}

int FreeList::size() {
  return 0;
}

int FreeList::count() {
  return 0;
}

int FreeList::free_count() {
  return 0;
}

int FreeList::pending_count() {
  return 0;
}

pgid_t FreeList::allocate(int n) {
  return 0;
}

void FreeList::free(txid_t txid, Page *p) {

}

bool FreeList::freed(pgid_t pgid) {
  return false;
}
