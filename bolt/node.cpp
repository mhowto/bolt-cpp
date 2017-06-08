#include "node.h"
#include "bucket.h"
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
    sz += elsz + inode->keySize + inode->valueSize;
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
  return this->bucket->node(this->inodes[index]->id, this);
}

int Node::childIndex(Node *child) {
  auto first =
      std::lower_bound(this->inodes.begin(), this->inodes.end(), *child);
  if (first == this->inodes.end()) {
    return -1;
  }
  if (::strcmp((*first)->key, child->key_) == 0) {
    return first - this->inodes.begin();
  }
  return -1;
}
