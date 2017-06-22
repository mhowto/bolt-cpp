#include "bucket.h"
#include "tx.h"

Bucket::Bucket(Tx *tx) : fillPercent(DefaultFillPercent), tx_(tx) {}

bool Bucket::writable() { return tx_->writable(); }

// TODO
Node *Bucket::node(pgid id, const Node *parent) { return nullptr; }