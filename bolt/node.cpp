#include "node.h"
#include "bucket.h"
#include "meta.h"
#include "page.h"
#include "tx.h"
#include <algorithm>
#include <cassert>
#include <iostream>

extern const size_t pageHeaderSize;
extern const size_t leafPageElementSize;
extern const size_t branchPageElementSize;

Node *Node::root() {
  if (this->parent_ == nullptr) {
    return this;
  }
  return this->parent_->root();
}

int Node::size() const {
  int sz = pageHeaderSize;
  int elsz = this->pageElementSize();
  for (auto &inode : this->inodes) {
    sz += elsz + inode.key.size() + inode.value.size();
  }
  return sz;
}

int Node::pageElementSize() const {
  return this->isLeaf_ ? leafPageElementSize : branchPageElementSize;
}

bool Node::sizeLessThan(int v) const { return this->size() < v; }

Node *Node::childAt(int index) const {
  if (this->isLeaf_) {
    std::cerr << "invalid childAt(" << index << ") on a leaf node";
    std::exit(1);
  }
  return this->bucket_->node(this->inodes[index].id, this);
}

int Node::childIndex(const Node *child) const {
  auto first =
      std::lower_bound(this->inodes.begin(), this->inodes.end(), child->key());
  if (first == this->inodes.end()) {
    return -1;
  }
  if ((*first) == child->key_) {
    return first - this->inodes.begin();
  }
  return -1;
}

int Node::numChildren() const { return this->inodes.size(); }

Node *Node::nextSibling() const {
  if (!this->parent_) {
    return nullptr;
  }
  int index = this->parent_->childIndex(this);
  if (index < 0 || index >= this->parent_->numChildren() - 1) {
    return nullptr;
  }
  return this->parent_->childAt(index + 1);
}

Node *Node::prevSibling() const {
  if (!this->parent_) {
    return nullptr;
  }
  int index = this->parent_->childIndex(this);
  if (index <= 0) {
    return nullptr;
  }
  return this->parent_->childAt(index - 1);
}

int Node::get(const Slice &key, std::string *value) const {
  if (!value) {
    std::cerr << " value is null\n";
    std::exit(1);
  }
  // Find insertion index.
  auto first = std::lower_bound(this->inodes.begin(), this->inodes.end(), key);
  // Add capacity and shift nodes if we don't have an exact match and need to
  // insert.
  bool exact =
      numChildren() > 0 && first != this->inodes.end() && (*first) == key;
  if (!exact) {
    return -1;
  }
  const INode &inode = *first;
  value->assign(inode.value.data(), inode.value.size());
  return 0;
}

// TODO: value may be null
void Node::put(const Slice &oldKey, const Slice &newKey, const Slice &value,
               pgid id, std::uint32_t flags) {
  const Meta *meta = this->bucket_->tx()->meta();
  if (!meta) {
    std::cerr << "meta is null\n";
    std::exit(1);
  }

  if (id >= meta->pgID()) {
    std::cerr << "pgid (" << id << ") above high water mark (" << meta->pgID()
              << ")";
    std::exit(1);
  } else if (oldKey.size() <= 0) {
    std::cerr << "put: zero-length old key\n";
    std::exit(1);
  } else if (newKey.size() <= 0) {
    std::cerr << "put: zero-length new key\n";
    std::exit(1);
  }

  // Find insertion index.
  auto first =
      std::lower_bound(this->inodes.begin(), this->inodes.end(), oldKey);
  // Add capacity and shift nodes if we don't have an exact match and need to
  // insert.
  bool exact =
      numChildren() > 0 && first != this->inodes.end() && (*first) == oldKey;
  auto index = first - inodes.begin();
  if (!exact) {
    this->inodes.insert(first, INode());
  }
  INode &inode = this->inodes[index];
  inode.flags = flags;
  inode.key = newKey;
  inode.value = value;
  inode.id = id;
  if (inode.key.size() <= 0) {
    std::cerr << "put: zero-length inode key\n";
    std::exit(-1);
  }
}

void Node::del(const Slice &key) {
  if (key.size() <= 0) {
    std::cerr << "del: zero-length key\n";
    std::exit(1);
  }
  auto first = std::lower_bound(this->inodes.begin(), this->inodes.end(), key);
  bool exact =
      numChildren() > 0 && first != this->inodes.end() && (*first) == key;
  if (!exact) {
    return;
  }
  // Delete inode from the node.
  this->inodes.erase(first);
  // Mark the node as needing rebalacing.
  this->unbalanced_ = true;
}

void Node::write(Page *p) {
  // initilize page's header
  if (this->isLeaf_) {
    p->setFlags(LeafPageFlag);
  } else {
    p->setFlags(BranchPageFlag);
  }
  if (this->inodes.size() > 0xffff) {
    std::cerr << "page's count is overflow: pgid = " << p->id();
    std::exit(1);
  }
  p->unsetOverflow();
  p->setCount(this->inodes.size());

  if (p->count() == 0) {
    return;
  }

  char *b = reinterpret_cast<char *>(
      p->ptr() + (this->inodes.size() * this->pageElementSize()));
  for (size_t i = 0; i < this->inodes.size(); ++i) {
    const INode &n = this->inodes[i];
    if (this->isLeaf_) {
      LeafPageElement *elem = p->leafPageElement(i);
      elem->flags = n.flags;
      elem->ksize = n.key.size();
      elem->vsize = n.value.size();
      elem->pos = static_cast<std::uint32_t>((char *)(b) - (char *)(elem));
    } else {
      BranchPageElement *elem = p->branchPageElement(i);
      elem->ksize = n.key.size();
      elem->id = n.id;
      elem->pos = static_cast<std::uint32_t>((char *)(b) - (char *)(elem));
      if (elem->id != p->id()) {
        std::cerr << "write: circular dependency occured\n";
        std::exit(1);
      }
    }

    std::memcpy(b, n.key.data(), n.key.size());
    b += n.key.size();
    std::memcpy(b, n.value.data(), n.value.size());
    b += n.value.size();
  }
}

void Node::read(Page *p) {
  // initilize node's header
  std::string s;
  this->id_ = p->id();
  this->isLeaf_ = (p->flags() & LeafPageFlag) ? true : false;
  this->inodes.clear();
  this->inodes.reserve(p->count());

  for (size_t i = 0; i < p->count(); i++) {
    INode inode;
    if (this->isLeaf_) {
      LeafPageElement *elem = p->leafPageElement(i);
      inode.flags = elem->flags;
      inode.key = elem->key();
      inode.value = elem->value();
    } else {
      BranchPageElement *elem = p->branchPageElement(i);
      INode inode;
      inode.id = elem->id;
      inode.key = elem->key();
    }
    if (inode.key.size() <= 0) {
      std::cerr << "read: zero-length inode key\n";
      std::exit(1);
    }
    this->inodes.push_back(std::move(inode));
  }

  // Save first key so we can find the node in the parent when we spill.
  if (this->inodes.size() > 0) {
    this->key_ = this->inodes[0].key;
    assert(this->key_.size() > 0);
  }
}
