#include "meta.h"
#include "bucket.h"
#include "cstdio"
#include "exception.h"
#include "molly/hash/hash.h"
#include "page.h"
#include <iostream>

namespace hash = molly::hash;

// Represents a marker value to indicate that a file is a Bolt DB.
const std::uint32_t Magic = 0xED0CDAED;

// The data file format version.
const int Version = 2;

void Meta::validate() {
  if (this->magic != Magic) {
    throw DatabaseInvalidException();
  } else if (this->version != Version) {
    throw DatabaseVersionMismatchException();
  } else if (this->checksum != 0 && this->checksum != this->sum64()) {
    throw DatabaseChecksumException();
  }
}

void Meta::write(Page *p) {
  if (this->root->root >= this->pgid) {
    std::cerr << "root bucket pgid (" << this->root->root << ") above high water mark (" << this->pgid << "\n";
    std::abort();
  } else if (this->freelist >= this->pgid) {
    std::cerr << "freelist pgid (" << this->freelist << ") above high water mark (" << this->pgid << ")\n";
    std::abort();
  }

  // page id is either going to be 0 or 1 which we can determine by the transaction ID.
  p->setID(this->txid % 2);
  p->setFlags(p->flags() | MetaPageFlag);

  // Calculate the checksum.
  this->checksum = this->sum64();
  *(p->meta()) = *this;
}

std::uint64_t Meta::sum64() {
  size_t first = reinterpret_cast<size_t>(&magic);
  size_t end = reinterpret_cast<size_t>(&checksum);
  return hash::fnva64_buf(this, end - first);
}