#ifndef __BOLT_CURSOR_H
#define __BOLT_CURSOR_H

#include "slice.h"
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

class Bucket;
class Page;
class Node;
typedef std::uint64_t pgid;

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
  // bucket returns the bucket that this cursor was created from.
  Bucket *bucket() { return bucket_; }

  // first moves the cursor to the first item in the bucket and returns its key
  // and value. If the bucket is empty then an empty key and value are returned.
  // The returned key and value are only valid for the life of the transaction.
  std::pair<std::optional<Slice>, std::optional<Slice>> first();

  // last moves the cursor to the last item in the bucket and returns its key
  // and value. If the bucket is empty then an empty key and value are returned.
  // The returned key and value are only valid for the life of the transaction.
  std::pair<std::optional<Slice>, std::optional<Slice>> last();

  // next moves the cursor to the next item in the bucket and returns its key
  // and value. If the bucket is at the end of the bucket then a nil key and
  // value are returned. The returned key and value are only valid for the life
  // of the transaction.
  std::pair<std::optional<Slice>, std::optional<Slice>> next();

  // prev moves the cursor to the previous item in the bucket and returns its
  // key and value. If the bucket is at the beginning of the bucket then a nil
  // key and value are returned. The returned key and value are only valid for
  // the life of the transaction.
  std::pair<std::optional<Slice>, std::optional<Slice>> prev();

  // seek moves the cursor to a given key and returns it.
  // If the key does not exist then the next key is used. If no keys follow, a
  // nil key is returned.
  // The returned key and value are only valid for the life of the transaction.
  std::pair<std::optional<Slice>, std::optional<Slice>> seek(const Slice &seek);

  // keyValue returns the key and value of the current leaf element.
  std::tuple<std::optional<Slice>, std::optional<Slice>, std::uint32_t>
  keyValue();

  // deleteCurrent removes the current key/value under the cursor from the
  // bucket.
  // deleteCurrent fails if current key/value is a bucket or if the
  // transaction
  // is not writable.
  void deleteCurrent();

private:
  // first_ moves the cursor to the first leaf element under the last page in
  // the stack.
  void first_();

  void last_();

  // next_ moves to the next leaf element and returns the key, value and flags.
  // If the cursor is at the last leaf element then it stays there and returns
  // nil.
  std::tuple<std::optional<Slice>, std::optional<Slice>, std::uint32_t> next_();

  // seek_ moves the cursor to a given key and returns it.
  // If the key down not exist then the next key is used.
  std::tuple<std::optional<Slice>, std::optional<Slice>, std::uint32_t>
  seek_(const Slice &seek);

  // search recursively performs a binary search against a given  page/node
  // until it finds a given key.
  void search(const Slice &key, pgid id);

  void searchNode(const Slice &key, Node *n);
  void searchPage(const Slice &key, Page *p);
  // nsearch searches the leaf node on the top of the stack for a key.
  void nsearch(const Slice &key);

  // node returns the code that the cursor is currently positioned on.
  Node *node();

  Bucket *bucket_;

  // stack stores the ref of the elements on the path.
  // ref0 -> ref1 -> ref2 -> ... -> refN
  // refJ is in the refJ-1.inodes[refJ-1.index]
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