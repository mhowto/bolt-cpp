#ifndef __BOLT_DB_H
#define __BOLT_DB_H

#include "meta.h"
#include "molly/os/file.h"
#include "page.h"
#include "stats.h"
#include <functional>
#include <gsl/gsl>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

using File = molly::os::File;

enum class byte : unsigned char {};
typedef int FileMode;

class Tx;
class Meta;
struct FreeList;

// Option represents the options that can be set when opening a database.
struct Option {
  // Timeout is the amount of time to wait to obtain a file lock.
  // When set to zero it will wait indefinitely. This option is only
  // available on Darwin and Linux.
  int Timeout;

  // Sets the DB.NoGroupSync flag before memory mapping the file
  bool NoGrowSync;

  // Open database in read-only mode. Uses flock(..., LOCK_S | LOCK_NB) to
  // grab a shared lock (UNIX).
  bool ReadOnly;

  // Sets the DB.MmapFlags flag before memory mapping the file.
  int MmapFlags;

  // InitialMmapSize is the initial mmap size of the database
  // in bytes. Read transactions won't block write transaction
  // if the InitialMmapSize is large enough to hold database mmap
  // size.
  //
  // If <= 0, the initial map size is 0.
  // If initialMmapSize is smaller than the previous database size,
  // it takes no effect;
  int InitialMmapSize;
};

// DB* open(std::string path, FileMode mode, Option* option);

class DB {
public:
  // open a database at the given path.
  // If the file does not exist then it will be created automatically.
  // Passing in nil options will cause bolt to open the database with the default options.
  DB(std::string path, FileMode mode, Option *option);
  ~DB();

  // page retrieves a page reference from the mmap based on the current page
  // size.
  Page *page(pgid_t id);

  // Update executes a function within the context of a read-write managed
  // transaction.
  // If no error is returned from the function then the trasaction is committed.
  // If an error is returned then the entire transaction is rolled back.
  // Any error that is returned from the function or returned from the commit is
  // returned from the update() method method.
  //
  // Attempting to manually commit or rollback within the function will cause a
  // panic.
  void update(std::function<void(Tx *)>);

  // start a new transaction.
  Tx *begin(bool writable);

  int fd();
  Meta *meta();

  bool read_only() { return read_only_; }

private:
  void close();

  Tx *begin_tx();
  Tx *begin_rwtx();
  void remove_tx(Tx *);
  void flock(int timeout);
  void funlock();
  void munmap();
  void init();

  template <class Container> Page *page_in_buffer(Container &buf, pgid_t id);

private:
  int pageSize_;
  bool opened_;

  // When enabled, the database will perform a Check() after every commit.
  // A panic is issued if the database is in an inconsistent state. This
  // flag has a large performace impact so it should only be used for
  // debugging purposee;
  bool strict_mode_;

  // Setting the no_sync flag will cause the database to skip fsync()
  // calls after ecah commit. This can be useful when bulk loading data
  // into a database and you can restart the bulk load in the event of
  // a system failure or database corruption. Do not set this flag
  // for normal use.
  //
  // If the package global IngoreNoSync constant is true, this value is
  // ignored. See the comment on that constant for more details.
  //
  // THIS IS UNSAFE. PLEASE USE WITH CAUTION.
  bool no_sync_;

  // When true, skips the truncate call when growing the database.
  // Setting this to true is only safe on non-ext3/ext4 systems.
  // Skipping truncation avoids preallocation of hard drive space and
  // bypasses a truncate() and fsync() syscall on remapping.
  //
  // https://github.com/boltdb/bolt/issues/284
  bool no_grow_sync_;

  // If you want to read the entire database fast, you can set mmap_falgs_ to
  // syscall.MAP_POPULATE on Linux 2.6.23+ for sequential read-ahead.
  int mmap_flags_;

  // max_batch_size_ is the maximum size of a batch. Default value is
  // copied from DefaultMaxbatchSize in constructor.
  //
  // If <= 0, disables batching.
  //
  // Do not change concurrently with calls to Batch.
  int max_batch_size_;

  // max_batch_delay_ is the maximum delay(in milliseconds) before a batch starts.
  // Default value is copied from DefaultMaxBatchDelay in constructor.
  //
  // If <=0, effectively disables batching.
  //
  // Do not change concurrently with calls to Batch.
  int max_batch_delay_;

  // alloc_size_ is the amount of space allocated when the database
  // needs to create new pages. This is done to amortize the cost
  // of truncate() and fsync() when growing the data file.
  int alloc_size_;

  std::string path_;
  gsl::owner<File *> file_;
  gsl::owner<File *> lock_file_; // windows only
  char *dataref_;
  char *data_; // pointer to mmapped  file
  int data_sz_;
  int file_sz_; // current on disk file size
  gsl::owner<Meta *> meta0;
  gsl::owner<Meta *> meta1;
  int page_size_;
  Tx *rwtx_;
  std::vector<Tx *> txs_;
  struct FreeList *freelist_;
  struct Stats stats_;

  mutable std::mutex rwlock_;          // Allows only one writer at a time.
  mutable std::mutex metalock_;        // Protects meta page access.
  mutable std::shared_mutex mmaplock_; // Protects mmap access during remapping.
  mutable std::shared_mutex statlock_; // Ptotects stat access.

  // Read only mode.
  // When true, Update() and Begin(true) return DatabaseReadOnlyException
  bool read_only_;
};

DB *open(std::string path, FileMode mode, Option *option);

template <class Container> Page *DB::page_in_buffer(Container &buf, pgid_t id) {
  return reinterpret_cast<Page *>(&buf[id * static_cast<pgid_t>(this->page_size_)]);
}

#endif