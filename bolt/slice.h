#ifndef __BOLT_SLICE_H
#define __BOLT_SLICE_H

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <tuple>

class Slice {
public:
  // Create an empty slice.
  Slice() : data_(""), size_(0) {}

  // Create a slice that refers to d[0, n-1]
  Slice(const char *d, size_t n) : data_(d), size_(n) {}

  // Create a slice that refers to s[0, strlen(s)-1]
  /* implicit */
  Slice(const char *s) : data_(s), size_(::strlen(s)) {}

  // Create a single slice from SliceParts using buf as storage.
  // buf must exist as long as the returned Slice exists.
  Slice(const struct SliceParts &parts, std::string *buf);

  // Return a pointer to the beginning of the referenced data
  const char *data() const { return data_; }

  // Return the length (in bytes) of the referenced data
  size_t size() const { return size_; }

  // Return true iff the length of the referenced data is zero
  bool empty() const { return size_ == 0; }

  // Return the ith byte in the referenced data.
  // REQUIRES: n < size()
  char operator[](size_t n) const {
    assert(n < size());
    return data_[n];
  }

  // Change this slice to refer to an empty array
  void clear() {
    data_ = "";
    size_ = 0;
  }

  // Drop the first "n" bytes from this slice.
  void remove_prefix(size_t n) {
    assert(n <= size());
    data_ += n;
    size_ -= n;
  }

  void remove_suffix(size_t n) {
    assert(n <= size());
    size_ -= n;
  }

  // Return a string that contains the copy of the referenced data.
  // when hex is true, returns a string of twice the length hex encoded (0-9A-F)
  std::string ToString(bool hex = false) const;

  // Decodes the current slice interpreted as an hexadecimal string into result,
  // if successful returns true, if this isn't a valid hex string (e.g not
  // coming from Slice::ToString(true)) DecodeHex returns false.
  // This slice is expected to have an even number of 0-9A-F characters
  // also accepts lowercase (a-f)
  std::tuple<std::string, bool> DecodeHex() const;

  // Three-way comparison. Returns value:
  // <  0 iff "*this" <  "that"
  // == 0 iff "*this" == "that"
  // >  0 iff "*this" >  "that"
  int compare(const Slice &that) const;

  // Return true iff "x" is a prefix of "*this"
  bool starts_with(const Slice &x) const { return ((size_ >= x.size_) && (std::memcmp(data_, x.data_, x.size_) == 0)); }

  bool ends_with(const Slice &x) const {
    return ((size_ >= x.size_) && (memcmp(data_ + size_ - x.size_, x.data_, x.size_) == 0));
  }

  // Compare two slices and returns the first byte where they differ
  size_t difference_offset(const Slice &that) const;

private:
  const char *data_;
  size_t size_;
};

inline int Slice::compare(const Slice &that) const {
  const size_t min_len = (size_ < that.size_) ? size_ : that.size_;
  int r = ::memcmp(data_, that.data_, min_len);
  if (r == 0) {
    if (size_ < that.size_) {
      r = -1;
    } else if (size_ > that.size_) {
      r = +1;
    }
  }
  return r;
}

inline size_t Slice::difference_offset(const Slice &that) const {
  size_t off = 0;
  const size_t len = (size_ < that.size_) ? size_ : that.size_;
  for (; off < len; off++) {
    if (data_[off] != that.data_[off])
      break;
  }
  return off;
}

// A set of Slices that are virtually concatenated together. 'parts'
// points to an array of Slices. The number of elements in the array
// is 'num_parts'.
struct SliceParts {
  SliceParts(const Slice *_parts, int _num_parts) : parts(_parts), num_parts(_num_parts) {}
  SliceParts() : parts(nullptr), num_parts(0) {}

  const Slice *parts;
  int num_parts;
};

inline bool operator==(const Slice &x, const Slice &y) {
  return ((x.size() == y.size()) && (std::memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator<(const Slice &x, const Slice &y) {
  size_t s1 = x.size();
  size_t s2 = y.size();
  size_t s = (s1 < s2) ? s1 : s2;
  int result = std::memcmp(x.data(), y.data(), s);
  return (result < 0) || (result == 0 && s1 < s2);
}

inline bool operator!=(const Slice &x, const Slice &y) { return !(x == y); }

#endif