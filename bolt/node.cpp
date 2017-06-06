#include "node.h"
#include "bucket.h"
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
