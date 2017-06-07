#include "slice.h"

Slice::Slice(const struct SliceParts &parts, std::string *buf) {
  size_t length = 0;
  for (int i = 0; i < parts.num_parts; ++i) {
    length += parts.parts[i].size();
  }
  buf->reserve(length);

  for (int i = 0; i < parts.num_parts; ++i) {
    buf->append(parts.parts[i].data(), parts.parts[i].size());
  }
  data_ = buf->data();
  size_ = buf->size();
}

// 2 small internal utility functions, for efficient hex conversions
// and no need for snprintf, toupper etc...
// Originally from wdt/util/EncryptionUtils.cpp - for ToString(true)/DecodeHex:
char toHex(unsigned char v) {
  if (v <= 9) {
    return '0' + v;
  }
  return 'A' + v - 10;
}
// most of the code is for validation/error check
int fromHex(char c) {
  // toupper:
  if (c >= 'a' && c <= 'f') {
    c -= ('a' - 'A'); // aka 0x20
  }
  // validation
  if (c < '0' || (c > '9' && (c < 'A' || c > 'F'))) {
    return -1; // invalid not 0-9A-F hex char
  }
  if (c <= '9') {
    return c - '0';
  }
  return c - 'A' + 10;
}

std::string Slice::ToString(bool hex) const {
  std::string result; // RVO/NRVO/move
  if (hex) {
    result.reserve(2 * size_);
    for (size_t i = 0; i < size_; ++i) {
      unsigned char c = static_cast<unsigned char>(data_[i]);
      result.push_back(toHex(c >> 4));
      result.push_back(toHex(c & 0xf));
    }
  } else {
    result.assign(data_, size_);
  }
  return result;
}

std::tuple<std::string, bool> Slice::DecodeHex() const {
  std::string result;
  std::string::size_type len = size_;
  if (len % 2) {
    goto failed;
  }
  for (size_t i = 0; i < len;) {
    int h1 = fromHex(data_[i++]);
    if (h1 < 0) {
      goto failed;
    }
    int h2 = fromHex(data_[i++]);
    if (h2 < 0) {
      goto failed;
    }
    result.push_back((h1 << 4) | h2);
  }
  return std::make_tuple(result, true);
failed:
  return make_tuple(std::string(), false);
}