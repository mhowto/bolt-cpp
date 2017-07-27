#ifndef __BOLT_DB_H
#define __BOLT_DB_H

#include <functional>
#include <gsl/gsl>
#include <mutex>
#include <shared_mutex>
#include <string>

enum class byte : unsigned char {};
typedef int FileMode;
typedef std::uint64_t pgid;

class Tx;

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

class Page;
namespace molly {
namespace os {
class File;
}
}

class DB {
public:
  DB(std::string path, FileMode mode, Option *option);
  ~DB();

  // page retrieves a page reference from the mmap based on the current page
  // size.
  Page *page(pgid id);

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

  std::string Path;

  int fd();
  // int LockFile; // winodws only
  // char *DataRef; // mmap'ed readonly, write throws SEGV
  byte *Data;
  // int MaxMapSize;
  int FileSize; // current on disk file size
  int DataSize;

  bool ReadOnly;

  int MapFlags = 0;

private:
  Tx *beginTx();
  Tx *beginRWTx();
  void removeTx(Tx *);

  gsl::owner<molly::os::File *> file_;
  int pageSize_;
  bool opened_;

  mutable std::mutex rwlock_;          // Allows only one writer at a time.
  mutable std::mutex metalock_;        // Protects meta page access.
  mutable std::shared_mutex mmaplock_; // Protects mmap access during remapping.
  mutable std::shared_mutex statlock_; // Ptotects stat access.
};

DB *open(std::string path, FileMode mode, Option *option);

#endif