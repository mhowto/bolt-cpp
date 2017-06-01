#ifndef __BOLT_PAGE_H
#define __BOLT_PAGE_H

#include <cstdint>
#include <string>
#include <vector>

typedef std::uint64_t pgid;
class Meta;
class LeafPageElement;
class BranchPageElement;

class Page {
public:
  // type returns a human readable page type string used for debugging.
  std::string type();
  // meta returns a pointer to the metadata section of the page.
  Meta *meta();

  // leafPageElement retrieves the leaf node by index.
  LeafPageElement *leafPageElement(std::uint16_t index);
  // leafPageElements retrieves a list of leaf nodes.
  std::vector<LeafPageElement> leafPageElements();

  // branchPageElement retrieves the brnach node by index.
  BranchPageElement *branchPageElement(std::uint16_t index);
  // branchPageElements retrieves a list of branch nodes.
  std::vector<BranchPageElement> branchPageElements();

  // dump writes n bytes of the page to STDERR as hex output.
  void hexdump(int n);

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
private:
    std::uint32_t flags;
    std::uint32_t pos;
};


#endif