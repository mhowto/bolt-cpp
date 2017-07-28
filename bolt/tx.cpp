#include "tx.h"
#include "bucket.h"
#include "db.h"
#include "meta.h"
#include "page.h"

Tx::Tx(DB *db, bool writable) : db_(db), writable_(writable) {
  this->meta_ = new Meta(*db->meta());

  // copy over the root bucket
  this->root_ = new Bucket(this);
  this->root_->set_bucket(*this->meta_->root());

  // Increment the transaction id and add a page cache for writable
  // transactions.
  if (this->writable_) {
    this->meta_->incrementTxID();
  }
}

Page *Tx::page(pgid id) {
  // Check the dirty pages first.
  auto search = this->pages_.find(id);
  if (search != this->pages_.end()) {
    return search->second;
  }

  // Otherwise return directly from the mmap.
  return this->db_->page(id);
}
