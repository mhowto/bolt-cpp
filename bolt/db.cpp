#include "db.h"
#include "exception.h"
#include "molly/os/file.h"
#include "tx.h"
#include "unistd.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>

// namespace os = molly::os;

DB::DB(std::string path, FileMode mode, Option *option) {
  this->Path = path;
  int flag = O_RDWR;
  if (option != nullptr && option->ReadOnly) {
    flag = O_RDONLY;
    this->ReadOnly = true;
  }

  file_ = new molly::os::File(path, flag | O_CREAT, mode | S_IRWXU);

  // Initialize the database if it doesn't exist
  // TODO: fix it
  this->pageSize_ = ::getpagesize();
}

Page *DB::page(pgid id) {
  int pos = id * this->pageSize_;
  return reinterpret_cast<Page *>(this->Data + pos);
}

Tx *DB::begin(bool writable) {
  if (writable) {
    return this->beginRWTx();
  }
  return this->beginTx();
}

Tx *DB::beginTx() {
  // Lock the meta pages while we initialize the transaction.We obtain
  // the meta lock before the mmap lock because that's the order that the
  // write transaction will obtain them.
  this->metalock_.lock();

  // Obtain a read-only lock on the mmap. When the mmap is remapped it will
  // obtain a write lock so all transactions must finish before it can be
  // remapped.
  this->mmaplock_.lock_shared();

  //  Exit if the database is not open yet.
  if (this->opened_) {
    this->mmaplock_.unlock_shared();
    this->metalock_.unlock();
    throw DatabaseNotOpenException();
  }

  // Create a transaction associated with the database.
  Tx *t = new Tx(this);

  // Keep track of transaction until it closes.

  // Unlock the meta pages.

  // Update the transaction stats.
  return t;
}

Tx *DB::beginRWTx() { return new Tx(this); }
// void removeTx(Tx *);

void DB::update(std::function<void(Tx *)> fn) {
  Tx *tx = this->begin(true);
  fn(tx);
}

DB::~DB() { delete file_; }

int DB::fd() { return file_->fd(); }

DB *open(std::string path, FileMode mode, Option *option) { return nullptr; }
