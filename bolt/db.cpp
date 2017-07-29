#include "db.h"
#include "exception.h"
#include "freelist.h"
#include "molly/os/file.h"
#include "tx.h"
#include "unistd.h"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>

// namespace os = molly::os;

DB::DB(std::string path, FileMode mode, Option *option) {
  this->Path = path;
  int flag = O_RDWR;
  if (option != nullptr && option->ReadOnly) {
    flag = O_RDONLY;
    this->read_only_ = true;
  }

  this->freelist_ = new struct FreeList();

  file_ = new molly::os::File(path, flag | O_CREAT, mode | S_IRWXU);

  // Initialize the database if it doesn't exist
  // TODO: fix it
  this->pageSize_ = ::getpagesize();
}

Page *DB::page(pgid_t id) {
  int pos = id * this->pageSize_;
  return reinterpret_cast<Page *>(this->Data + pos);
}

Tx *DB::begin(bool writable) {
  if (writable) {
    return this->begin_rwtx();
  }
  return this->begin_tx();
}

Tx *DB::begin_tx() {
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
  this->txs_.push_back(t);

  // Unlock the meta pages.
  this->metalock_.unlock();

  // Update the transaction stats.
  this->statlock_.lock();
  this->stats_.tx_n++;
  this->stats_.open_tx_n = this->txs_.size();
  this->statlock_.unlock();
  return t;
}

Tx *DB::begin_rwtx() {
  // If the database was opened with Options.ReadOnly, return an error.
  if (this->read_only_) {
    throw DatabaseReadOnlyException();
  }

  // obtain writer lock. This is released by the transaction when it closes.
  // This enforces only one writer transaction at a time.
  this->rwlock_.lock();

  // Once we have the writer lock then we can lock the meta pages so that we can
  // set up the transaction.
  std::lock_guard<std::mutex> metalock(this->metalock_);

  // Exit if the database is not open yet.
  if (this->opened_) {
    this->rwlock_.unlock();
    throw DatabaseNotOpenException();
  }

  // Create a transaction associated with the database.
  Tx *t = new Tx(this, true);
  this->rwtx_ = t;

  // Free any pages associated with closed read-only transactions.
  txid_t minid = 0xFFFFFFFFFFFFFFFF;
  for (auto &tx : this->txs_) {
    if (tx->meta()->txid() < minid) {
      minid = tx->meta()->txid();
    }
  }
  if (minid > 0) {
    this->freelist_->release(minid - 1);
  }

  return t;
}
// void removeTx(Tx *);

void DB::update(std::function<void(Tx *)> fn) {
  Tx *tx = this->begin(true);
  fn(tx);
}

DB::~DB() { delete file_; }

int DB::fd() { return file_->fd(); }

DB *open(std::string path, FileMode mode, Option *option) { return nullptr; }
