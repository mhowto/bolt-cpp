
#include "cursor.h"
#include "bucket.h"
#include "node.h"
#include "page.h"
#include "tx.h"
#include <cassert>

bool elemRef::isLeaf() {
  if (this->node) {
    return this->node->isLeaf();
  }
  return (this->page->flags() & LeafPageFlag) != 0;
}

int elemRef::count() {
  if (this->node) {
    return this->node->numChildren();
  }
  return static_cast<int>(this->page->count());
}

std::optional<std::pair<Slice, Slice>> Cursor::first() {
  this->bucket_->tx()->db();
  //    ->get();
  //    db();
  //    assert(this->bucket_->tx()->db() != nullptr);
}