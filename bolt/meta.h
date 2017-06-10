#ifndef __BOLT_META_H
#define __BOLT_META_H

#include <cstdint>

class Bucket;

typedef std::uint64_t pgid;
typedef std::uint64_t txid;

class Meta {
public:
  pgid pgID() const { return id_; }
  txid txID() const { return txid_; }

  std::uint32_t magic;
  std::uint32_t version;
  std::uint32_t pageSize;
  std::uint32_t flags;
  Bucket *root;
  pgid freelist;
  pgid id_;
  txid txid_;
  std::uint64_t checksum;
};

#endif