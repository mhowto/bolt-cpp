#ifndef __BOLT_TX_H
#define __BOLT_TX_H

#include "gsl/gsl"
#include "meta.h"
#include <cstdint>
#include <map>

class DB;
class Bucket;
class Page;

// Tx represents a read-only or read/write transaction on the database.
// Read-only transactions can be used for retrieving values for keys and
// creating cursors.
// Read/write transactions can create and remove buckets and create and remove
// keys.
class Tx {
public:
  Tx(DB *db, bool writable = false);
  const Meta *meta() const { return meta_; }
  const Bucket *root() const { return root_; }

  // writeFlag specifies the flag for write-related methods like WriteTo().
  // Tx opens the database file with the specified flag to copy the data.
  //
  // By default, the flag is unset, which works well for mostly in-memory
  // workloads. For databases that are much larger than availabe RAM, set
  // the flag to syscall.O_DIRECT to avoid trashing the page cache.
  int writeFlag;

  bool writable() const { return writable_; }
  bool managed() const { return managed_; }

  DB *db() { return db_; }

  // page returns a pointer to the page with a given id.
  // If page has been written to then a temporary buffered page is returned.
  Page *page(pgid_t id);

private:
  bool writable_;
  bool managed_;
  DB *db_;
  gsl::owner<Meta *> meta_;
  gsl::owner<Bucket *> root_;
  std::map<pgid_t, Page *> pages_;
  // TxStats stats;
  // std::vector<> commitHandlers;
};

#endif