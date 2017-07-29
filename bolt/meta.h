#ifndef __BOLT_META_H
#define __BOLT_META_H

#include "types.h"
#include <cstdint>

struct bucket;
class Page;

class Meta {
public:
  pgid_t pgid() const { return id_; }
  txid_t txid() const { return txid_; }

  // validate checks the marker bytes and version of the meta page to ensure it
  // matches the binary. Throw exception if validation failed.
  void validate();

  // writes the meta onto a page
  void write(Page *page);

  // generates the checksum for the meta
  std::uint64_t sum64();

  void incrementTxID() { txid_ += 1; }

  const struct bucket *root() const { return root_; }

private:
  std::uint32_t magic_;
  std::uint32_t version_;
  std::uint32_t pageSize_;
  std::uint32_t flags_;
  struct bucket *root_;
  pgid_t freelist_;
  pgid_t id_;
  txid_t txid_;
  std::uint64_t checksum_;
};

#endif