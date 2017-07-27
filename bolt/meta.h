#ifndef __BOLT_META_H
#define __BOLT_META_H

#include <cstdint>

struct bucket;
class Page;

typedef std::uint64_t pgid;
typedef std::uint64_t txid;

class Meta {
public:
  pgid pgID() const { return id_; }
  txid txID() const { return txid_; }
  Meta(const Meta *m) : Meta(*m) {}

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
  pgid freelist_;
  pgid id_;
  txid txid_;
  std::uint64_t checksum_;
};

#endif