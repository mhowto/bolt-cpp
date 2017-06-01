#ifndef __BOLT_DB_H
#define __BOLT_DB_H

#include <string>

enum class byte : unsigned char {};
typedef int FileMode;

// Option represents the options that can be set when opening a database.
struct Option{
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

//DB* open(std::string path, FileMode mode, Option* option);

class DB {
public:
  DB(std::string path, FileMode mode, Option* option);

  std::string Path;
  int FileDescriptor;
  // int LockFile; // winodws only
  // char *DataRef; // mmap'ed readonly, write throws SEGV
  byte *Data;
  // int MaxMapSize;
  int FileSize; // current on disk file size
  int DataSize;

  bool ReadOnly;

  int MapFlags = 0;
};

#endif