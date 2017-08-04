#ifndef __BOLT_META_H
#define __BOLT_META_H

#include "types.h"
#include <cstdint>

struct bucket;
class Page;

class Meta {
public:
  // validate checks the marker bytes and version of the meta page to ensure it
  // matches the binary. Throw exception if validation failed.
  void validate();

  // writes the meta onto a page
  void write(Page *page);

  // generates the checksum for the meta
  std::uint64_t sum64();

  void incrementTxID() { txid += 1; }

public:
  std::uint32_t magic;
  std::uint32_t version;
  std::uint32_t page_size;
  std::uint32_t flags;
  struct bucket *root;
  pgid_t freelist;
  pgid_t pgid;
  txid_t txid;
  std::uint64_t checksum;
};

#endif