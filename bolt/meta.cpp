#include "meta.h"
#include "bucket.h"
#include "exception.h"
#include "molly/hash/hash.h"
#include "page.h"

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

// writes the meta onto a page
void Meta::write(Page *page) {}

// generates the checksum for the meta
std::uint64_t Meta::sum64() {
  size_t first = reinterpret_cast<size_t>(&magic);
  size_t end = reinterpret_cast<size_t>(&checksum);
  return hash::fnva64_buf(this, end - first);
}