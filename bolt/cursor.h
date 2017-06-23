#ifndef __BOLT_CURSOR_H
#define __BOLT_CURSOR_H

class Bucket;
class Page;
class Node;

// Cursor represents an iterator that can traverse over all key/value pairs in a
// bucket in sorted order.
// Cursors see nested buckets with value == nil.
// Cursors can be obtained from a transaction and are valid as long as the
// transaction is open.
//
// Keys and values returned from the cursor are only valid for the life of the
// transaction.
//
// Changing data while traversing with a cursor may cause it to be invalidated
// and return unexpected keys and/or values. You must reposition your cursor
// after mutating data.
class Cursor {
public:
private:
  Bucket *bucket_;
  std::vector<struct elemRef> stack_;
};

// elemRef represents a reference to an element on a given page/node.
struct elemRef {
  Page *page;
  Node *node;
  int index;

  // isLeaf returns whether the ref is pointing at a leaf page/node.
  bool isLeaf();

  // count returns the number of inodes or page elements.
  int count();
};

#endif