#ifndef __BOLT_BUCKET_H
#define __BOLT_BUCKET_H

#include "gsl/gsl"
#include "types.h"
#include <cstdint>
#include <map>
#include <string>

class Node;
class Tx;
class Page;
class Cursor;
class Slice;

// DefaultFillPercent is the percentage that split pages are filled.
// This value can be changed by setting Bucket.FillPercent.
const double DefaultFillPercent = 0.5;

// bucket represents the on-file representation of a bucket.
// This is stored as the "value" of a bucket key. If the bucket is small enough,
// then its root page can be stored inline in the "value", after the bucket
// header. In the case of inline buckets, the "root" will be 0.
struct bucket {
  pgid_t root;
  std::uint64_t sequence;
};

// Bucket represents a collection of key/value pairs inside the database.
class Bucket {
public:
  Bucket(Tx *tx);

  void set_bucket(const struct bucket &b);
  // node creates a node from a page and associates it with a given parent.
  Node *node(pgid_t id, const Node *parent);

  gsl::not_null<Tx *> tx();
  //  { return tx_; }

  // root returns the root of the bucket.
  pgid_t root() { return bucket_.root; }

  // writable returns whether the bucket is writable.
  bool writable();

  // cursor creates a cursor associated with the bucket.
  // The cursor is only valid as long as the transaction is open.
  // Do not use a cursor after the transaction is closed.
  Cursor *cursor();

  // bucket returns a nested bucket by name.
  // Returns nil if the  bucket does not exist.
  // The bucket instance is only valid for the lifetime of the transaction.
  Bucket *bucket(Slice name);

  // createBucket creates a new bucket at the given key and returns the new
  // bucket.
  // Returns an error if the key already exists, if the bucket name is blank, or
  // if the
  // bucket name is too long.
  // The bucket instance is only valid for the lifetime of the transaction.
  Bucket *create_bucket(Slice key);

  Bucket *create_bucket_if_not_exists(Slice key);

  void delete_bucket(Slice key);

  Slice get(Slice key) const;

  void put(Slice key, Slice value);

  void delete_by_key(Slice key);

  // sequence returns the current integer for the bucket without incrementing
  // it.
  std::uint64_t sequence();

  // setSequence updates the sequence number for the bucket.
  void set_sequence(std::uint64_t v);

  // nextSequence returns an autoincrementing integer for the bucket.
  std::uint64_t next_sequence();

  // pageNode returns the in-memory node, if it exists.
  // Otherwises returns the underlying page.
  std::pair<Page *, Node *> page_node(pgid_t id);

  // inline_ checks whether the bucket is inline
  bool inline_();

  // Sets the threshold for filling nodes when they split. By default,
  // the bucket will fill to 50% but it can be useful to increase this
  // amount if you know that your write workloads are mostly append-only.
  //
  // This is non-persisted across transactions so it must be set in every
  // Tx.
  double fillPercent;

private:
  // Helper method that re-interprets a sub-bucket value from
  // a parent into a Bucket.
  Bucket *open_bucket(Slice value);

  struct bucket bucket_;
  gsl::not_null<Tx *> tx_;                  // the associated transaction
  std::map<std::string, Bucket *> buckets_; // subbucket cache
  Page *page;                               // inline page reference
  Node *rootNode;                 // materialized node for the root page
  std::map<pgid_t, Node *> nodes; // node cache
};

#endif