#include "bucket.h"
#include "tx.h"

Bucket::Bucket(Tx *tx) : fillPercent(DefaultFillPercent), tx_(tx) {}

bool Bucket::writable() { return tx_->writable(); }

gsl::not_null<Tx *> Bucket::tx() { return tx_; }

// TODO
Node *Bucket::node(pgid id, const Node *parent) { return nullptr; }