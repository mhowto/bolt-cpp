#ifndef __BOLT_NODE_H
#define __BOLT_NODE_H

#include "slice.h"
#include <cstdint>
#include <string>
#include <vector>

class Bucket;
struct INode;

typedef std::uint64_t pgid;

// Node represents an in-memory, deserialized page.
class Node {
public:
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

  // put inserts a key/value
  void put(const Slice &oldKey, const Slice &newKey, const Slice &value,
           pgid id, std::uint32_t flags);

  // del removes a key from the node.
  void del(const Slice &key);

private:
  Bucket *bucket;
  bool isLeaf;
  bool unbalanced;
  bool spilled;
  char *key;
  pgid id;
  Node *parent;
  std::vector<Node *> children;
  std::vector<INode *> inodes;
};

// INode represents an internal node inside of a node.
// It can be used to point to elements in a page or
// point to an element which hasn't been added to a page yet.
struct INode {
  std::uint32_t flags;
  pgid id;
  char *key;
  int keySize;
  char *value;
  int valueSize;
};

#endif