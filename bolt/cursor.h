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

  std::tuple<std::optional<Slice>, std::optional<Slice>, int> keyValue();

  // deleteCurrent removes the current key/value under the cursor from the
  // bucket.
  // deleteCurrent fails if current key/value is a bucket or if the
  // transaction
  // is not writable.
  void deleteCurrent();

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