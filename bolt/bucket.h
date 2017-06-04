#ifndef __BOLT_BUCKET_H
#define __BOLT_BUCKET_H

#include <map>

class Node;
typedef std::uint64_t pgid;

// Bucket represents a collection of key/value pairs inside the database.
class Bucket {
public:
  std::map<pgid, Node *> nodes;
};

#endif