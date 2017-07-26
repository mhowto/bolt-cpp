#include "db.h"
#include "molly/os/file.h"
#include "unistd.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>

// namespace os = molly::os;

DB::DB(std::string path, FileMode mode, Option *option) {
  this->Path = path;
  int flag = O_RDWR;
  if (option != nullptr && option->ReadOnly) {
    flag = O_RDONLY;
    this->ReadOnly = true;
  }

  file_ = new molly::os::File(path, flag | O_CREAT, mode | S_IRWXU);

  // Initialize the database if it doesn't exist
  // TODO: fix it
  this->pageSize_ = ::getpagesize();
}

Page *DB::page(pgid id) {
  int pos = id * this->pageSize_;
  return reinterpret_cast<Page *>(this->Data + pos);
}

DB::~DB() { delete file_; }

int DB::fd() { return file_->fd(); }

DB *open(std::string path, FileMode mode, Option *option) { return nullptr; }
