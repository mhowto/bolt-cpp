#ifndef __BOLT_TX_H
#define __BOLT_TX_H

#include "meta.h"
#include "molly/os/file.h"
#include "slice.h"
#include "stats.h"
#include <gsl/gsl>
#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

class DB;
class Page;
class Cursor;
class Bucket;
struct PageInfo;

namespace os = molly::os;

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
  const TxStats &stats() { return stats_; }

  // bucket retrieves a bucket by name.
  // Returns null if the bucket does not exist.
  // The bucket instance is only valid for the lifetime of the transaction.
  Bucket *bucket(Slice name);

  // create_bucket creates a new bucket.
  // Returns an error if the bucket already exists, if the bucket name is blank, or if the bucket name is too long.
  // The bucket instance is only valid for the lifetime of the transction.
  Bucket *create_bucket(Slice name);

  // create_bucket_if_not_exists creates a new bucket if it doesn't already exist.
  // Returns an error if the bucket name is blank, or if the bucket name is too long.
  // The bucket instance is only valid for the lifetime of the transaction.
  Bucket *create_bucket_if_not_exists(Slice name);

  void delete_bucket(Slice name);

  // for_each executes a function for each bucket in the root.
  // If the provided function returns an error then the iteration is stopped
  // and the error is returned to the caller.
  void for_each(std::function<void(Slice name, Bucket *b)> fn);

  // on_commit adds a handler function to be executed after the transaction successfully commits.
  void on_commit(std::function<void()> fn);

  // commit writes all changes to disk and updates the meta page.
  // Returns an error if a disk write error occurs, or if Commit is
  // called on a read-only transaction.
  void commit();

  // Rollback closes the transction and igores all previous updates. Read-only
  // transcations must be rolled back and not commited.
  void rollback();

  // Write_to writes the entire database to a writer.
  // If err == nil then exactly tx.Size() bytes will be written into the writer.
  std::int64_t write_to(os::File w);

  // copy_file copies the entire database to file at the given path.
  // A reader transaction is maintained during the copy so it is safe to continue
  // using the database while a copy is in progress.
  void copy_file(std::string path, os::file_mode mode);

  // Check performs several consistency checks on the database for this transaction.
  // An error is returned if any inconsistency is found.
  //
  // It can be safely run concurrently on a writable transaction. However, this
  // incurs a high cost for large databases and databases with a lot of subbuckets
  // because of caching. This overhead can be removed if running on a read-only
  // transaction, however, it is not safe to execute other writer transactions at
  // the same time.
  void check();

  // page returns page information for a given page number.
  // This is only safe for concurrent use when used by a writable transaction.
  PageInfo *page(int id);

private:
  bool writable_;
  bool managed_;
  DB *db_;
  gsl::owner<Meta *> meta_;
  gsl::owner<Bucket *> root_;
  std::map<pgid_t, Page *> pages_;
  TxStats stats_;
  std::vector<std::function<void()> > commit_handlers_;

  void _rollback();

  void close();

  void check_bucket(Bucket *b, std::map<pgid_t, Page *> reachable, std::set<pgid_t> freed);

  // allocate returns a contiguous block of memory starting at a given page.
  Page *allocate(int count);

  // write writes any dirty pages to disk.
  void write();

  // writeMeta writes the meta to the disk
  void write_meta();

  // page returns a reference to the page with a given id.
  // If page has been written to then a temporary buffered page is returned.
  Page *_page(pgid_t id);

  // for_each_page iterates over every page within a given page and executes a function.
  void for_each_page(pgid_t pgid, int depth, std::function<void(Page *, int)> fn);
};

#endif