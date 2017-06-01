#include "page.h"
#include "meta.h"

#include <iostream>
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

LeafPageElement* Page::leafPageElement(std::uint16_t index) const {
    LeafPageElement* ptr = reinterpret_cast<LeafPageElement*>(this->ptr);
    return ptr + index;
}

std::vector<LeafPageElement> Page::leafPageElements() const {
    std::vector<LeafPageElement> result;
    if(this->count == 0)  {
        return result;
    }
    LeafPageElement* ptr = reinterpret_cast<LeafPageElement*>(this->ptr);
    for (int i = 0; i < this->count; i++) {
        result.push_back(*(ptr+i));
    }
    return result;
}

BranchPageElement* Page::branchPageElement(std::uint16_t index) const {
    BranchPageElement* ptr = reinterpret_cast<BranchPageElement*>(this->ptr);
    return ptr + index;
}

std::vector<BranchPageElement> Page::branchPageElements() const {
    std::vector<BranchPageElement> result;
    if(this->count == 0)  {
        return result;
    }
    BranchPageElement* ptr = reinterpret_cast<BranchPageElement*>(this->ptr);
    for (int i = 0; i < this->count; i++) {
        result.push_back(*(ptr+i));
    }
    return result;
}

void Page::hexdump(int n) const {
    char* buf = reinterpret_cast<char*>(const_cast<Page*>(this));
    std::string s (buf, n);
    std::cerr << s << '\n';
}