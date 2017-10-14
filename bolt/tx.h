#ifndef __BOLT_TX_H
#define __BOLT_TX_H

#include "gsl/gsl"
#include "meta.h"
#include <cstdint>
#include <map>
#include <string>

class DB;
class Bucket;
class Page;
class Cursor;
class TxStats;

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

  // id returns the transaction id.
  txid_t id() { return this->meta_->txid; }

  // db returns a reference to the database that created the transaction.
  DB *db() { return db_; }

  // size returns current database size in bytes as seen by this transaction.
  std::int64_t size();

  // page returns a pointer to the page with a given id.
  // If page has been written to then a temporary buffered page is returned.
  Page *page(pgid_t id);

  // cursor creates a cursor associated with the root bucket.
  // All items in the cursor will return a nil value becaurse all root bucket keys point to buckets.
  // The cursor is noly valid as long as the transaction is open.
  // Do not use a cursor after the transaction is closed;
  Cursor *cursor();

  // stats retrieves a copy of the current transaction statistics.
  TxStats *stats();

  // bucket retrieves a bucket by name.
  // Returns null if the bucket does not exist.
  // The bucket instance is only valid for the lifetime of the transaction.
  Bucket *bucket(std::string name);

  // createBucket creates a new bucket.
  // Returns an error if the bucket already exists, if the bucket name is blank, or if the bucket name is too long.
  // The bucket instance is only valid for the lifetime of the transction.
  Bucket *createBucket(std::string name);

  // createBucketIfNotExists creates a new bucket if it doesn't already exist.
  // Returns an error if the bucket name is blank, or if the bucket name is too long.
  // The bucket instance is only valid for the lifetime of the transaction.
  Bucket *createBucketIfNotExists(std::string name);

  void deleteBucket(std::string name);

  // forEach executes a function for each bucket in the root.

  // onCommit adds a handler function to be executed after the transaction successfully commits.

  // commit writes all changes to disk and updates the meta page.
  // Returns an error if a disk write error occurs, or if Commit is
  // called on a read-only transaction.
  void commit();

  // Rollback closes the transction and igores all previous updates. Read-only
  // transcations must be rolled back and not commited.
  void rollback();

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