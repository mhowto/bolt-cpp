#include "tx.h"
#include "meta.h"

Tx::Tx(DB *db) : db_(db) {
  this->meta_ = new Meta();

  // TODO: refactor
  // mock for node test
  this->meta_->id_ = 7;
  this->meta_->txid_ = 7;
}