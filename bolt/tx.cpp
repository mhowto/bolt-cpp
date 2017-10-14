#include "tx.h"
#include "bucket.h"
#include "db.h"
#include "meta.h"
#include "page.h"

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

void Tx::for_each(std::function<void(Slice name, Bucket *b)> fn) {}

void Tx::on_commit(std::function<void()> fn) {}

void Tx::commit() {}

void Tx::rollback() {}

std::int64_t Tx::write_to() { return 0; }

void Tx::copy_file(std::string path, os::file_mode mode) {}

void Tx::check() {}

PageInfo *Tx::page(int id) { return nullptr; }

void Tx::_rollback() {}

void Tx::close() {}

void Tx::check_bucket(Bucket *b, std::map<pgid_t, Page *> reachable, std::set<pgid_t> freed) {}

Page *Tx::allocate(int count) { return nullptr; }

void Tx::write() {}

void Tx::write_meta() {}

Page *Tx::_page(pgid_t id) { return nullptr; }

void Tx::for_each_page(pgid_t pgid, int depth, std::function<void(Page *, int)> fn) {}