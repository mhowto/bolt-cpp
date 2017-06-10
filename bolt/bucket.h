#ifndef __BOLT_BUCKET_H
#define __BOLT_BUCKET_H

#include "gsl/gsl"
#include <cstdint>
#include <map>
#include <string>

class Node;
class Tx;
class Page;
typedef std::uint64_t pgid;

// bucket represents the on-file representation of a bucket.
// This is stored as the "value" of a bucket key. If the bucket is small enough,
// then its root page can be stored inline in the "value", after the bucket
// header. In the case of inline buckets, the "root" will be 0.
struct bucket {
  pgid root;
  std::uint64_t sequence;
};

// Bucket represents a collection of key/value pairs inside the database.
class Bucket {
public:
  Bucket(Tx *tx);
  // node creates a node from a page and associates it with a given parent.
  Node *node(pgid id, const Node *parent);

  gsl::not_null<Tx *> tx() { return tx_; }

  // Sets the threshold for filling nodes when they split. By default,
  // the bucket will fill to 50% but it can be useful to increase this
  // amount if you know that your write workloads are mostly append-only.
  //
  // This is non-persisted across transactions so it must be set in every Tx.
  double fillPercent;

private:
  gsl::owner<bucket *> bucket_;
  Tx *tx_;                                  // the associated transaction
  std::map<std::string, Bucket *> buckets_; // subbucket cache
  Page *page;                               // inline page reference
  Node *rootNode;               // materialized node for the root page
  std::map<pgid, Node *> nodes; // node cache
};

#endif