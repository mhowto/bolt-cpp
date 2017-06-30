#include "db.h"
#include "unistd.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>

DB::DB(std::string path, FileMode mode, Option *option) {
  this->Path = path;
  int flag = O_RDWR;
  if (option != nullptr && option->ReadOnly) {
    flag = O_RDONLY;
    this->ReadOnly = true;
  }

  int fileDescriptor;
  if ((fileDescriptor = ::open(path.c_str(), flag | O_CREAT, mode | S_IRWXU)) ==
      -1) {
    char err[255];
    sprintf(err, "failt to create db file: %s", std::strerror(errno));
    throw std::runtime_error(err);
  } else {
    this->FileDescriptor = fileDescriptor;
  }

  // Initialize the database if it doesn't exist
  // TODO: fix it
  this->pageSize_ = ::getpagesize();
}

Page *DB::page(pgid id) {
  int pos = id * this->pageSize_;
  return reinterpret_cast<Page *>(this->Data + pos);
}
