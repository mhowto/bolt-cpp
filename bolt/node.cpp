#include "node.h"
#include "bucket.h"
#include "meta.h"
#include "tx.h"
#include <algorithm>
#include <iostream>

extern const size_t pageHeaderSize;
extern const size_t leafPageElementSize;
extern const size_t branchPageElementSize;

Node *Node::root() {
  if (this->parent == nullptr) {
    return this;
  }
  return this->parent->root();
}

int Node::size() const {
  int sz = pageHeaderSize;
  int elsz = this->pageElementSize();
  for (auto &inode : this->inodes) {
    sz += elsz + inode.keySize + inode.valueSize;
  }
  return sz;
}

int Node::pageElementSize() const {
  return this->isLeaf ? leafPageElementSize : branchPageElementSize;
}

bool Node::sizeLessThan(int v) const { return this->size() < v; }

Node *Node::childAt(int index) const {
  if (this->isLeaf) {
    std::cerr << "invalid childAt(" << index << ") on a leaf node";
    std::exit(1);
  }
  return this->bucket->node(this->inodes[index].id, this);
}

int Node::childIndex(Node *child) const {
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

void Node::put(const Slice &oldKey, const Slice &newKey, const Slice &value,
               pgid id, std::uint32_t flags) {
  const Meta *meta = this->bucket->tx()->meta();
  if (meta == nullptr) {
    std::cerr << "meta is null";
    std::exit(1);
  }

  if (id >= meta->pgID()) {
    std::cerr << "pgid (" << id << ") above high water mark (" << meta->pgID()
              << ")";
    std::exit(1);
  } else if (oldKey.size() <= 0) {
    std::cerr << "put: zero-length old key";
    std::exit(1);
  } else if (newKey.size() <= 0) {
    std::cerr << "put: zero-length new key";
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
  inode.key = newKey.data();
  inode.value = value.data();
  inode.id = id;
  if (::strlen(inode.key) <= 0) {
    std::cerr << "put: zero-length inode key\n";
    std::exit(-1);
  }
}
