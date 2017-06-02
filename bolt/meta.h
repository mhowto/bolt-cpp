#ifndef __BOLT_META_H
#define __BOLT_META_H

#include <cstdint>

class Bucket;

typedef std::uint64_t pgid;
typedef std::uint64_t txid;

class meta {
public:
private:
    std::uint32_t magic;
    std::uint32_t version;
    std::uint32_t pageSize;
    std::uint32_t flags;
    Bucket* root;
    pgid freelist;
    pgid pgID;
    txid txID;
    std::uint64_t checksum;
};

#endif