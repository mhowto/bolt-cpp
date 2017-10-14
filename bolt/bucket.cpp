#include "bucket.h"
#include "tx.h"
#include <iostream>

Bucket::Bucket(Tx *tx) : fillPercent(DefaultFillPercent), tx_(tx) {}

void Bucket::set_bucket(const struct bucket &b) {
  this->bucket_.root = b.root;
  this->bucket_.sequence = b.sequence;
}

bool Bucket::writable() { return tx_->writable(); }

gsl::not_null<Tx *> Bucket::tx() { return tx_; }

// TODO
Node *Bucket::node(pgid_t id, const Node *parent) { return nullptr; }

bool Bucket::inline_() { return this->bucket_.root == 0; }

std::pair<Page *, Node *> Bucket::page_node(pgid_t id) {
  // Inline buckets have a fake page embedded in their value so treat them
  // differently. We'll return the rootNode (if available) or the fake
  // paeg.
  if (inline_()) {
    if (id != 0) {
      std::cerr << "inline bucket non-zero page access(2): " << id << " != 0\n";
      std::exit(1);
    }

    if (this->rootNode) {
      //   return {nullptr, this->rootNode};
      return std::make_pair(nullptr, this->rootNode);
    }
    return std::make_pair(this->page, nullptr);
  }

  // Check the node cache for non-inline buckets.
  auto search = this->nodes.find(id);
  if (search != this->nodes.end()) {
    return std::make_pair(nullptr, search->second);
  }
  return std::make_pair(this->tx_->page(id), nullptr);
}

Bucket *Bucket::create_bucket(Slice name) { return nullptr; }
