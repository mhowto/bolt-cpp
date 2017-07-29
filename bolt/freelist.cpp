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
  auto middle = this->ids.end();
  this->ids.insert(this->ids.end(), m.begin(), m.end());
  std::inplace_merge(this->ids.begin(), this->ids.begin() + raw_size, this->ids.end());
}