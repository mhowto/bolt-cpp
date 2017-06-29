
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
  this->first_();

  // If we land on an empty page then move to the next value.
  // https://github.com/boltdb/bolt/issues/450
  if (this->stack_.back().count() == 0) {
    this->next_();
  }

  auto[k, v, flags] = this->keyValue();
  if ((flags & BucketLeafFlag) != 0) {
    return std::make_pair(k, std::optional<Slice>());
  }
  return std::make_pair(k, v);
}

std::tuple<std::optional<Slice>, std::optional<Slice>, std::uint32_t>
Cursor::keyValue() {
  auto &ref = this->stack_.back();
  if (ref.count() == 0 || ref.index >= ref.count()) {
    return std::make_tuple(std::optional<Slice>(), std::optional<Slice>(), 0);
  }

  // retrieve value from node
  if (ref.node) {
    INode &inode = ref.node->inodes[ref.index];
    return std::make_tuple(inode.key, inode.value, inode.flags);
  }

  // or retrieve value from page
  LeafPageElement *elem = ref.page->leafPageElement(ref.index);
  return std::make_tuple(elem->key(), elem->value(), elem->flags);
}

void Cursor::first_() {
  for (;;) {
    // Exit when we hit a leaf page
    auto &ref = this->stack_.back();
    if (ref.isLeaf()) {
      break;
    }

    // Keep adding pages pointing to the first elements to the stack.
    pgid id = (ref.node) ? ref.node->inodes[ref.index].id
                         : ref.page->branchPageElement(ref.index)->id;
    auto[p, n] = this->bucket_->pageNode(id);
    elemRef elem;
    elem.node = n;
    elem.page = p;
    elem.index = 0;
    this->stack_.push_back(std::move(elem));
  }
}

std::tuple<std::optional<Slice>, std::optional<Slice>, std::uint32_t>
Cursor::next_() {
  assert(this->stack_.size() > 0);
  for (;;) {
    // attempt to move over one element until we're successful.
    // Move up the stack as we hit the end of each page in our stack.
    auto iter = this->stack_.rbegin();
    for (; iter < this->stack_.rend(); ++iter) {
      if (iter->index < iter->count() - 1) {
        iter->index++;
        break;
      }
    }

    // If we've hit the root page then stop and return. This will leave the
    // cursor on the last element of the last page.
    if (iter == this->stack_.rend()) {
      return std::make_tuple(std::optional<Slice>(), std::optional<Slice>(), 0);
    }

    // Otherwise start from where we left off in the stack and find the first
    // element of the first leaf page.
    this->stack_.erase(this->stack_.begin() + (this->stack_.rend() - iter),
                       this->stack_.end());
    this->first_();

    // If this is an empty page then restart and move back up the stack.
    // https://github.com/boltdb/bolt/issues/450
    if (this->stack_.back().count() == 0) {
      continue;
    }

    return this->keyValue();
  }
}