
#include "cursor.h"
#include "bucket.h"
#include "node.h"
#include "page.h"
#include "tx.h"
#include <cassert>
#include <utility>

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

std::pair<std::optional<Slice>, std::optional<Slice>> Cursor::first() {
  assert(this->bucket_->tx()->db() != nullptr);
  this->stack_.clear();
  auto[p, n] = this->bucket_->pageNode(this->bucket_->root());

  elemRef elem;
  elem.page = p;
  elem.node = n;
  elem.index = 0;
  this->stack_.push_back(std::move(elem));
  this->first();

  // If we land on an empty page then move to the next value.
  // https://github.com/boltdb/bolt/issues/450
  if (this->stack_.back().count() == 0) {
    this->next();
  }

  auto[k, v, flags] = this->keyValue();
  if ((flags & BucketLeafFlag) != 0) {
    return std::make_pair(k, std::optional<Slice>());
  }
  return std::make_pair(k, v);
}