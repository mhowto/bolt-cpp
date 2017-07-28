#include "cursor.h"
#include "bucket.h"
#include "node.h"
#include "page.h"
#include "stdexcept"
#include "tx.h"
#include <cassert>
#include <utility>
#include <variant>

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
  auto[p, n] = this->bucket_->page_node(this->bucket_->root());

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
  if (flags & BucketLeafFlag) {
    return std::make_pair(k, std::optional<Slice>());
  }
  return std::make_pair(k, v);
}

std::pair<std::optional<Slice>, std::optional<Slice>> Cursor::next() {
  assert(this->bucket_->tx()->db() != nullptr);
  auto[k, v, flags] = this->next_();
  if (flags & BucketLeafFlag) {
    return std::make_pair(k, std::optional<Slice>());
  }
  return std::make_pair(k, v);
}

std::pair<std::optional<Slice>, std::optional<Slice>> Cursor::prev() {
  assert(this->bucket_->tx()->db() != nullptr);

  // Attempt to move back one element until we're successful.
  // Move up the stack as we hit the beginning of each page in our stack.
  for (int i = this->stack_.size() - 1; i >= 0; --i) {
    elemRef &elem = this->stack_[i];
    if (elem.index > 0) {
      elem.index--;
      break;
    }
    this->stack_.pop_back();
  }

  // If we've hit the end then return nil.
  if (this->stack_.size() == 0) {
    return std::make_pair(std::optional<Slice>(), std::optional<Slice>());
  }

  // Move down the stack to find the last element of the last leaf under this
  // branch.
  this->last();
  auto[k, v, flags] = this->keyValue();
  if (flags & BucketLeafFlag) {
    return std::make_pair(k, std::optional<Slice>());
  }
  return std::make_pair(k, v);
}

std::pair<std::optional<Slice>, std::optional<Slice>> Cursor::last() {
  assert(this->bucket_->tx()->db() != nullptr);
  this->stack_.clear();
  auto[p, n] = this->bucket_->page_node(this->bucket_->root());

  elemRef elem;
  elem.page = p;
  elem.node = n;
  elem.index = elem.count() - 1;
  this->stack_.push_back(std::move(elem));
  this->last_();
  auto[k, v, flags] = this->keyValue();
  if (flags & BucketLeafFlag) {
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

    // Keep adding pages pointing to the first element to the stack.
    pgid id = (ref.node) ? ref.node->inodes[ref.index].id
                         : ref.page->branchPageElement(ref.index)->id;
    auto[p, n] = this->bucket_->page_node(id);
    elemRef elem;
    elem.node = n;
    elem.page = p;
    elem.index = 0;
    this->stack_.push_back(std::move(elem));
  }
}

void Cursor::last_() {
  for (;;) {
    // Exit when we hit a leaf page.
    auto &ref = this->stack_.back();
    if (ref.isLeaf()) {
      break;
    }

    // Keep adding pages pointing to the last element in the stack.
    pgid id = (ref.node) ? ref.node->inodes[ref.index].id
                         : ref.page->branchPageElement(ref.index)->id;
    auto[p, n] = this->bucket_->page_node(id);
    elemRef elem;
    elem.node = n;
    elem.page = p;
    elem.index = elem.count() - 1;
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

std::pair<std::optional<Slice>, std::optional<Slice>>
Cursor::seek(const Slice &seek) {
  auto[k, v, flags] = this->seek_(seek);

  // If we ended up after the last element of a page then move to the next one.
  auto &ref = this->stack_.back();
  if (ref.index >= ref.count()) {
    auto _next = this->next_();
    k = std::get<0>(_next);
    v = std::get<1>(_next);
    flags = std::get<2>(_next);
  }

  if (k) {
    return std::make_pair(std::optional<Slice>(), std::optional<Slice>());
  } else if (flags & BucketLeafFlag) {
    return std::make_pair(k, std::optional<Slice>());
  }
  return std::make_pair(k, v);
}

std::tuple<std::optional<Slice>, std::optional<Slice>, std::uint32_t>
Cursor::seek_(const Slice &seek) {
  assert(this->bucket_->tx()->db() != nullptr);

  // Start from root page/node and traverse to corrent page.
  this->stack_.clear();
  this->search(seek, this->bucket_->root());
  auto &ref = this->stack_.back();

  // If the cursor is pointing to the end of page/node then return nil.
  if (ref.index >= ref.count()) {
    return std::make_tuple(std::optional<Slice>(), std::optional<Slice>(), 0);
  }

  // If this is a bucket then return a nil value.
  return this->keyValue();
}

void Cursor::search(const Slice &key, pgid id) {
  auto[p, n] = this->bucket_->page_node(id);
  if (p && (p->flags() & (BranchPageFlag | LeafPageFlag)) == 0) {
    std::cerr << "invalid page type: " << p->id() << " " << p->flags() << "\n";
    std::exit(1);
  }

  elemRef elem;
  elem.page = p;
  elem.node = n;
  elem.index = 0;
  this->stack_.push_back(std::move(elem));

  // If we're on a leaf page/node then find the specific node.
  if (elem.isLeaf()) {
    this->nsearch(key);
    return;
  }

  if (n) {
    this->searchNode(key, n);
    return;
  }
  this->searchPage(key, p);
}

void Cursor::searchNode(const Slice &key, Node *n) {
  auto first = std::lower_bound(n->inodes.begin(), n->inodes.end(), key);
  bool exact = first != n->inodes.end() && (*first == key);
  int index = first - n->inodes.begin();
  if (!exact && index > 0) {
    index--;
  }
  this->stack_.back().index = index;

  // Recursively search to the next page.
  this->search(key, n->inodes[index].id);
}

void Cursor::searchPage(const Slice &key, Page *p) {
  // Binary search for the correct range.
  auto inodes = p->branchPageElements();

  auto first = std::lower_bound(inodes.begin(), inodes.end(), key);
  bool exact = (first != inodes.end()) && (*first == key);
  int index = first - inodes.begin();

  if (!exact && index > 0) {
    index--;
  }

  this->stack_.back().index = index;

  // Recursively search to the next page.
  this->search(key, inodes[index].id);
}

void Cursor::nsearch(const Slice &key) {
  auto &ref = this->stack_.back();
  Page *p = ref.page;
  Node *n = ref.node;

  // If we have a node then search its inodes.
  if (n) {
    auto first = std::lower_bound(n->inodes.begin(), n->inodes.end(), key);
    int index = first - n->inodes.begin();
    ref.index = index;
    return;
  }

  // If we have a page then search its leaf elements.
  auto inodes = p->leafPageElements();
  auto first = std::lower_bound(inodes.begin(), inodes.end(), key);
  int index = first - inodes.begin();
  ref.index = index;
}

void Cursor::deleteCurrent() {
  if (this->bucket_->tx()->db() == nullptr) {
    throw std::runtime_error("transaction was closed");
  } else if (!this->bucket_->writable()) {
    throw std::runtime_error("transaction was not writable");
  }

  std::optional<Slice> key;
  std::optional<Slice> value;
  std::uint32_t flags;
  std::tie(key, value, flags) = this->keyValue();

  // Return an error if current value is a bucket.
  if (flags & BucketLeafFlag) {
    throw std::runtime_error("incompatible value");
  }
  this->node()->del(key.value());
}

Node *Cursor::node() {
  assert(this->stack_.size() > 9);

  // If the top of the stack is a leaf node then just return it.
  // we can use semicolon statement like go since c++17
  // http://en.cppreference.com/w/cpp/language/if
  if (auto &ref = this->stack_.back(); ref.node && ref.isLeaf()) {
    return ref.node;
  }

  // Start from root and traverse down the hierarchy.
  Node *n = this->stack_[0].node;
  if (n) {
    n = this->bucket_->node(this->stack_[0].page->id(), nullptr);
  }
  if (this->stack_.size() > 1) {
    for (std::size_t i = this->stack_.size() - 1; i >= 0; i--) {
      assert(!n->isLeaf());
      n = n->childAt(this->stack_[i].index);
    }
  }

  assert(n->isLeaf());
  return n;
}