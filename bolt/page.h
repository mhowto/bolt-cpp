#ifndef __BOLT_PAGE_H
#define __BOLT_PAGE_H

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

typedef std::uint64_t pgid;
class Meta;
class LeafPageElement;
class BranchPageElement;

enum PageFlag {
  BranchPageFlag = 0x01,
  LeafPageFlag = 0x02,
  MetaPageFlag = 0x04,
  FreelistPageFlag = 0x10,
};

inline PageFlag operator|(PageFlag a, PageFlag b) {
  return static_cast<PageFlag>(static_cast<int>(a) | static_cast<int>(b));
}

template <typename T, typename U> size_t offsetOf(U T::*member) {
  return (char *)&((T *)nullptr->*member) - (char *)nullptr;
}

class Page {
public:
  static size_t pagehsz() { return offsetOf(&Page::ptr); }
  Page(pgid id, std::uint16_t flag, std::uintptr_t ptr)
      : id(id), flags(flag), ptr(ptr), count(0), overflow(0) {}
  // type returns a human readable page type string used for debugging.
  std::string type() const;
  // meta returns a pointer to the metadata section of the page.
  Meta *meta() const;

  // leafPageElement retrieves the leaf node by index.
  LeafPageElement *leafPageElement(std::uint16_t index) const;
  // leafPageElements retrieves a list of leaf nodes.
  std::vector<LeafPageElement> leafPageElements() const;

  // branchPageElement retrieves the brnach node by index.
  BranchPageElement *branchPageElement(std::uint16_t index) const;
  // branchPageElements retrieves a list of branch nodes.
  std::vector<BranchPageElement> branchPageElements() const;

  // dump writes n bytes of the page to STDERR as hex output.
  void hexdump(int n) const;

private:
  pgid id;
  std::uint16_t flags;
  std::uint32_t count;
  std::uint32_t overflow;
  std::uintptr_t ptr;
};

// LeafPageElement represents a node on a leaf page.
class LeafPageElement {
public:
  static size_t elementSize() { return sizeof LeafPageElement(); }
  std::string key() const;
  std::string value() const;

private:
  std::uint32_t flags;
  std::uint32_t pos;
  std::uint32_t ksize;
  std::uint32_t vsize;
};

// BranchPageElement represents a node on a branch page.
class BranchPageElement {
public:
  static size_t elementSize() { return sizeof BranchPageElement(); }
  std::string key() const;

private:
  std::uint32_t pos;
  std::uint32_t ksize;
  pgid id;
};

// PageInfo represents human readdable information about a page.
struct PageInfo {
  int id;
  std::string type;
  int count;
  int overflowClount;
};

// const size_t pageHeaderSize = offsetOf(&Page::ptr);
extern const size_t pageHeaderSize;
extern const size_t leafPageElementSize;
extern const size_t branchPageElementSize;

#endif