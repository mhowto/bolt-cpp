#include "tx.h"
#include "bucket.h"
#include "db.h"
#include "meta.h"
#include "page.h"
#include "exception.h"
#include "freelist.h"

Tx::Tx(DB *db, bool writable) : writable_(writable), db_(db) {
  // Copy the meta page since it can be changed by the writer.
  this->meta_ = new Meta(*db->meta());

  // Copy over the root bucket.
  this->root_ = new Bucket(this);
  this->root_->set_bucket(*this->meta_->root);

  // Increment the transaction id and add a page cache for writable
  // transactions.
  if (this->writable_) {
    this->meta_->incrementTxID();
  }
}

Page *Tx::page(pgid_t id) {
  // Check the dirty pages first.
  auto search = this->pages_.find(id);
  if (search != this->pages_.end()) {
    return search->second;
  }

  // Otherwise return directly from the mmap.
  return this->db_->page(id);
}

std::int64_t Tx::size() { return static_cast<std::int64_t>(this->meta_->pgid) * this->db_->page_size(); }

Cursor *Tx::cursor() { return root_->cursor(); }

Bucket *Tx::bucket(Slice name) { return root_->bucket(name); }

Bucket *Tx::create_bucket(Slice name) { return root_->create_bucket(name); }

Bucket *Tx::create_bucket_if_not_exists(Slice name) { return root_->create_bucket_if_not_exists(name); }

void Tx::delete_bucket(Slice name) { return root_->delete_bucket(name); }

void Tx::for_each(std::function<void(Slice name, Bucket *b)> fn) {
  root_->for_each([this, fn=fn](Slice k, Slice v) {
    fn(k, root_->bucket(k));
  });
}

void Tx::on_commit(std::function<void()> fn) { commit_handlers_.push_back(fn); }

void Tx::commit() {
  assert(!managed_);
  if (!db_) {
    throw TxClosedException();
  }
  if (!writable_) {
    throw TxNotWritableException();
  }

  // TODO: Use vectorized I/O to write out dirty pages.

  // Rebalance nodes which have had deletions

  // spill data onto dirty pages

  // free the old root bucket

  // free the freelist and allocate new pages for it. This will overestimate
  // the size of the freelist but not underestimate the size (which would be bad).

  // If the high water mark has moved up then attempt to grow the database.

  // Write dirty pages to disk.

  // If strict mode is enabled then perform a consistency check.
  // Only the first consistency error is reported in the panic.

  // Write meta to disk.

  // Finalize the transaction.

  // Execute commit handlers now that the locks have been removed.
}

void Tx::rollback() {
  assert(!managed_);
  if (!db_) {
    throw TxClosedException();
  }
  _rollback();
}

void Tx::_rollback() {
  if (!db_) {
    return;
  }
  if (writable_) {
    db_->freelist_->rollback(meta_->txid);
    db_->freelist_->reload(db_->page(db_->meta()->freelist));
  }
  close();
}

void Tx::close() {
  if (!db_) {
    return;
  }
  if (writable_) {
    // Grab freelist stats.
    auto freelist_free_n = db_->freelist_->free_count();
    auto freelist_pending_n = db_->freelist_->pending_count();
    auto freelist_alloc = db_->freelist_->size();

    // Remove transaction ref & writer lock.
    db_->rwtx_ = nullptr;
    db_->rwlock_.unlock();

    // Merge statistics.
    db_->statlock_.lock();
    db_->stats_.free_page_n = freelist_free_n;
    db_->stats_.pending_page_n = freelist_pending_n;
    db_->stats_.free_alloc =  (freelist_free_n + freelist_pending_n) * db_->page_size();
    db_->stats_.freelist_inuse = freelist_alloc;
    db_->stats_.tx_stats += stats_;
    db_->statlock_.unlock();
  } else {
    db_->remove_tx(this);
  }

  // clear all references.
  db_ = nullptr;
  delete meta_;
  delete root_;
  pages_.clear();
}

std::int64_t Tx::write_to(os::File w) {
  // Attempt to open reader with WriteFlag
  os::File* f = os::open_file(db_->path_, os::OPEN_RDONLY|writeFlag, os::ModeDefault);

  return 0;
}

void Tx::copy_file(std::string path, os::file_mode mode) {}

void Tx::check() {}

PageInfo *Tx::page(int id) { return nullptr; }

void Tx::check_bucket(Bucket *b, std::map<pgid_t, Page *> reachable, std::set<pgid_t> freed) {}

Page *Tx::allocate(int count) { return nullptr; }

void Tx::write() {}

void Tx::write_meta() {}

Page *Tx::_page(pgid_t id) { return nullptr; }

void Tx::for_each_page(pgid_t pgid, int depth, std::function<void(Page *, int)> fn) {}