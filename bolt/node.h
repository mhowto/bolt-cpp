#ifndef __BOLT_NODE_H
#define __BOLT_NODE_H

#include "slice.h"
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

class Bucket;
class Page;

typedef std::uint64_t pgid;

// INode represents an internal node inside of a node.
// It can be used to point to elements in a page or
// point to an element which hasn't been added to a page yet.
struct INode {
  std::uint32_t flags;
  pgid id;
  Slice key;
  Slice value;
};

inline bool operator<(const INode &lhs, const INode &rhs) {
  return lhs.key < rhs.key;
}

inline bool operator<(const INode &n, const Slice &key) { return n.key < key; }

inline bool operator==(const INode &n, const Slice &key) {
  return n.key == key;
}

inline bool operator==(const INode &n, const char *key) { return n.key == key; }

// Node represents an in-memory, deserialized page.
class Node {
public:
  Node(Bucket *bucket, bool isLeaf, Node *parent)
      : bucket_(bucket), isLeaf_(isLeaf), parent_(parent) {}

  Slice key() const { return key_; }

  // root returns the top-level node this node is attached to.
  Node *root();

  // minKeys returns the minimum number of inodes this node should have.
  int minKeys();

  // size returns the size of the node after serialization.
  int size() const;

  // sizeLessThan returns true if the node is less than a given size.
  // This is an optimization to avoid calculating a large node when we
  // only need to know if it fits inside a certain page size.
  bool sizeLessThan(int v) const;

  // pageElementSize returns the size of each page element based on the type of
  // node.
  int pageElementSize() const;

  // childAt returns the child node at a given index.
  Node *childAt(int index) const;

  // childIndex returns the index of a given child node.
  int childIndex(const Node *child) const;

  // numChildren returns the number of children.
  int numChildren() const;

  // nextSibling returns the next node with the same parent.
  Node *nextSibling() const;

  // prevSibling returns the previous node with the same parent.
  Node *prevSibling() const;

  // get queries a value
  int get(const Slice &key, std::string *value) const;

  // put inserts a key/value
  void put(const Slice &oldKey, const Slice &newKey, const Slice &value,
           pgid id, std::uint32_t flags);

  // del removes a key from the node.
  // not thread-safe
  void del(const Slice &key);

  // write writes the items onto one or more pages.
  // The caller is responsible for allocating a page with enough memory to store
  // the node.
  void write(Page *p);

  // read initializes the node from a page.
  void read(Page *p);

  // spill writes the nodes to dirty pages and splits nodes as it goes.
  // Throws an runtime_error if dirty pages cannot be allocated.
  void spill();

  // rebalance attempts to combine the node with sibling nodes if the node's
  // filled size is below a threshold or if there are not enough keys.
  void rebalance();

  // removes a node from the list of in-memory children.
  // This does not affect the inodes.
  void removeChild(Node *node);

  // dereference causes the node to copy all its inode key/value references to
  // heap memory. This is required when the mmap is reallocated so inodes are
  // not pointing to stale data.
  void dereference();

  // free adds the node's underlying page to thre freelist.
  void free();

  friend bool operator<(const Node &lhs, const Node &rhs) {
    return lhs.key_ < rhs.key_;
  }

  bool spilled() const { return spilled_; }
  bool isLeaf() const { return isLeaf_; }
  bool unbalanced() const { return unbalanced_; }
  pgid id() const { return id_; }

private:
  // split breaks up a node into multiple smaller nodes, if appropriate.
  // This should only be called from the spill() function.
  std::vector<Node *> split(int pageSize);

  // splitTwo breaks up a node into two smaller nodes, if appropriate.
  // This should only be called from the split() function.
  std::pair<Node *, Node *> splitTwo(int pageSize);

  // splitIndex finds the position where a page will fill a given threshold.
  // It returns the index as well as the size of the first page.
  // This is noly be called from split().
  std::pair<int, int> splitIndex(int threshold);

  Bucket *bucket_;
  bool isLeaf_;
  bool unbalanced_;
  bool spilled_;
  Slice key_;
  pgid id_;
  Node *parent_;
  std::vector<Node *> children; // help to record sub node during spilling
  std::vector<INode> inodes;
};

#endif
