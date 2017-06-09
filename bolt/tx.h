#ifndef __BOLT_TX_H
#define __BOLT_TX_H

#include <gsl/gsl>

class DB;
class Meta;
class Bucket;

// Tx represents a read-only or read/write transaction on the database.
// Read-only transactions can be used for retrieving values for keys and
// creating cursors.
// Read/write transactions can create and remove buckets and create and remove
// keys.
class Tx {
public:
  const Meta *meta() { return meta_; }

  // writeFlag specifies the flag for write-related methods like WriteTo().
  // Tx opens the database file with the specified flag to copy the data.
  //
  // By default, the flag is unset, which works well for mostly in-memory
  // workloads. For databases that are much larger than availabe RAM, set
  // the flag to syscall.O_DIRECT to avoid trashing the page cache.
  int writeFlag;

private:
  bool writable;
  bool managed;
  DB *db;
  Meta *meta_;
  gsl::owner<Bucket *> root;
  std::map<pgid, Page *> pages;
  // TxStats stats;
  // std::vector<> commitHandlers;
};

#endif