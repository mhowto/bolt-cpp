#include "page.h"
#include "meta.h"

#include <sstream>

std::string LeafPageElement::key() const {
    char* buf = reinterpret_cast<char*>(const_cast<LeafPageElement*>(this));
    return std::string(buf + this->pos, this->ksize);
}

std::string LeafPageElement::value() const {
    char* buf = reinterpret_cast<char*>(const_cast<LeafPageElement*>(this));
    return std::string(buf + this->pos + this->ksize, this->vsize);
}

std::string BranchPageElement::key() const {
    char* buf = reinterpret_cast<char*>(const_cast<BranchPageElement*>(this));
    return std::string(buf + this->pos, this->ksize);
}

std::string Page::type() const {
    if (this->flags & static_cast<int>(PageFlag::BranchPageFlag)) {
        return std::string("branch");
    } else if (this->flags & static_cast<int>(PageFlag::LeafPageFlag)) {
        return std::string("leaf");
    } else if (this->flags & static_cast<int>(PageFlag::MetaPageFlag)) {
        return std::string("meta");
    } else if (this->flags & static_cast<int>(PageFlag::FreelistPageFlag)) {
        return std::string("freelist");
    }
    std::ostringstream stringStream;
    stringStream << "unknown<" << this->flags << ">";
    return stringStream.str();
}

Meta* Page::meta() const {
    return reinterpret_cast<Meta*>(this->ptr);
}

/*
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
*/