#ifndef __BOLT_FREELIST_H
#define __BOLT_FREELIST_H

#include "types.h"
#include <map>
#include <set>
#include <vector>

class Page;

// freelist represents a list of all pages that are available for allocation.
// It also tracks pages that have been freed but are still in use by open
// transactions.
struct FreeList {
  std::vector<pgid_t> ids;                       // all free and available free page ids
  std::map<txid_t, std::vector<pgid_t>> pending; // mapping of soon-to-be free page ids by tx
  std::set<pgid_t> cache;                        // fast lookup of all free and pending page ids

  // size returns the size of the page after serialization.
  int size();

  // count returns count of pages on the freelist
  int count();

  // free_count returns count of free pages
  int free_count();

  // free_count returns count of pending pages
  int pending_count();

  // returns all free pgids (including all free ids and all pending ids) in one sorted list.
  std::vector<pgid_t> all_free_pgids();

  // allocate a contiguous list of pages of a given size. Returns the starting page id. If a contiguous block cannot be
  // found then 0 is returned.
  pgid_t allocate(int n);

  // free releases a page and its overflow for a given transaction id.
  // If the page is already free then a panic will occur.
  void free(txid_t txid, Page *p);

  // release moves all page ids for a transaction id (or older) to the freelist.
  void release(txid_t txid);

  // rollback removes the pages from a given pending tx.
  void rollback(txid_t txid);

  // freed returns whether a given page is in the free list.
  bool freed(pgid_t pgid);

  // read initializes the freelist from a freelist page.
  void read(Page *p);

  // write writes the page ids onto a freelist page. All free and pending ids are saved to disk
  // since in the event of a program crash, all pending ids will become free.
  void write(Page *p);

  // reload reads the freelist from a page and filters oput pending items.
  void reload(Page *p);

  // reindex rebuilds the free cache based on available and pending free lists.
  void reindex();
};

#endif