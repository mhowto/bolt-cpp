#include "page.h"
#include "meta.h"

#include <iostream>
#include <sstream>

extern const size_t pageHeaderSize = Page::pagehsz();
extern const size_t leafPageElementSize = LeafPageElement::elementSize();
extern const size_t branchPageElementSize = BranchPageElement::elementSize();

Slice LeafPageElement::key() const {
  char *buf = reinterpret_cast<char *>(const_cast<LeafPageElement *>(this));
  return Slice(buf + this->pos, this->ksize);
}

Slice LeafPageElement::value() const {
  char *buf = reinterpret_cast<char *>(const_cast<LeafPageElement *>(this));
  return Slice(buf + this->pos + this->ksize, this->vsize);
}

Slice BranchPageElement::key() const {
  char *buf = reinterpret_cast<char *>(const_cast<BranchPageElement *>(this));
  return Slice(buf + this->pos, this->ksize);
}

std::string Page::type() const {
  if (this->flags_ & static_cast<int>(PageFlag::BranchPageFlag)) {
    return std::string("branch");
  } else if (this->flags_ & static_cast<int>(PageFlag::LeafPageFlag)) {
    return std::string("leaf");
  } else if (this->flags_ & static_cast<int>(PageFlag::MetaPageFlag)) {
    return std::string("meta");
  } else if (this->flags_ & static_cast<int>(PageFlag::FreelistPageFlag)) {
    return std::string("freelist");
  }
  std::ostringstream stringStream;
  stringStream << "unknown<" << this->flags_ << ">";
  return stringStream.str();
}

Meta *Page::meta() const { return reinterpret_cast<Meta *>(this->ptr_); }

LeafPageElement *Page::leafPageElement(std::uint16_t index) const {
  LeafPageElement *ptr = reinterpret_cast<LeafPageElement *>(this->ptr_);
  return ptr + index;
}

std::vector<LeafPageElement> Page::leafPageElements() const {
  std::vector<LeafPageElement> result;
  if (this->count_ == 0) {
    return result;
  }
  LeafPageElement *ptr = reinterpret_cast<LeafPageElement *>(this->ptr_);
  for (unsigned int i = 0; i < this->count_; i++) {
    result.push_back(*(ptr + i));
  }
  return result;
}

BranchPageElement *Page::branchPageElement(std::uint16_t index) const {
  BranchPageElement *ptr = reinterpret_cast<BranchPageElement *>(this->ptr_);
  return ptr + index;
}

std::vector<BranchPageElement> Page::branchPageElements() const {
  std::vector<BranchPageElement> result;
  if (this->count_ == 0) {
    return result;
  }
  BranchPageElement *ptr = reinterpret_cast<BranchPageElement *>(this->ptr_);
  for (unsigned int i = 0; i < this->count_; i++) {
    result.push_back(*(ptr + i));
  }
  return result;
}

void Page::hexdump(int n) const {
  char *buf = reinterpret_cast<char *>(const_cast<Page *>(this));
  std::stringstream ss;
  ss << "0x";
  for (int i = 0; i < n; ++i)
    ss << std::hex << static_cast<int>(buf[i]);
  std::cerr << ss.str() << '\n';
}