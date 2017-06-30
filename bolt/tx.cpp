#include "tx.h"
#include "db.h"
#include "meta.h"
#include "page.h"

Tx::Tx(DB *db) : db_(db) {
  this->meta_ = new Meta();

  // TODO: refactor
  // mock for node test
  this->meta_->id_ = 7;
  this->meta_->txid_ = 7;
}

Page *Tx::page(pgid id) {
  // Check the dirty pages first.
  auto search = this->pages.find(id);
  if (search != this->pages.end()) {
    return search->second;
  }

  // Otherwise return directly from the mmap.
  return this->db_->page(id);
}
