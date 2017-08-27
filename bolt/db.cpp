#include "db.h"
#include "exception.h"
#include "freelist.h"
#include "meta.h"
#include "tx.h"
#include "unistd.h"
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <sys/file.h>
#include <sys/mman.h>
#include <thread>

namespace os = molly::os;

const int DefaultMaxBatchSize = 1000;
const int DefaultMaxBatchDelay = 10;
const int DefaultAllocSize = 16 * 1024 * 1024;

Option DefaultOption = {/* .Timeout */ 0, /* .NoGrowSync */ false, /* .ReadOnly */ false, /* .MmapFlags */ 0,
                        /* .InitialMmapSize */ 0};

DB::DB(std::string path, FileMode mode, Option *option) : opened_(false), path_(path) {
  // Set default option if no option is provided.
  if (!option) {
    option = &DefaultOption;
  }
  this->no_grow_sync_ = option->NoGrowSync;
  this->mmap_flags_ = option->MmapFlags;

  // Set default values for later DB operations.
  this->max_batch_size_ = DefaultMaxBatchSize;
  this->max_batch_delay_ = DefaultMaxBatchDelay;
  this->alloc_size_ = DefaultAllocSize;

  int flag = O_RDWR;
  if (option != nullptr && option->ReadOnly) {
    flag = O_RDONLY;
    this->read_only_ = true;
  }

  // open data file and separate sync handler for metadata writes.
  try {
    this->file_ = new File(path_, flag | O_CREAT, mode | S_IRWXU);
  } catch (std::exception &e) {
    this->close();
    throw e;
  }

  // Lock file so that other processes using Bolt in read-write mode cannot
  // use the database at the same time. This would cause corruption since
  // the two processes would write meta pages and free pages separately.
  // The database file is locked exclusively (only one process can grab the lock)
  // if !option.ReadOnly
  // The database file is locked using the shared lock (more than one process may
  // hold a lock at the same time) otherwise (option.ReadOnly is set).
  try {
    this->flock(option->Timeout);
  } catch (std::exception &e) {
    this->close();
    throw e;
  }

  // Default values for test hooks
  // directly use file->writeat

  // Initialize the database if it doesn't exist.
  os::file_info fi = this->file_->stat();
  if (fi.size == 0) {
    // Initialize new files with meta pages.
    this->init();
  } else {
    // Read the first meta page to determine the page size.
    std::string buf;
    buf.reserve(1000);
    this->file_->read_at(buf, 0);
    Meta *m = this->page_in_buffer(buf, 0)->meta();
    try {
      m->validate();
      this->page_size_ = m->page_size;
    } catch (std::exception &e) {
      this->page_size_ = ::getpagesize();
    }
  }

  // Initialize page pool.
  this->page_pool_ = new PagePool([s = pageSize_]() {
    char *bytes = new char[s];
    return Slice(bytes, s);
  });

  // memory map the data file.
  try {

  } catch (std::exception &e) {
    throw e;
  }

  // read in the freelist
  this->freelist_ = new struct FreeList();
  // this->freelist_->read(this->page(this->meta()->freelist()))
}

void DB::init() {
  // Set the page size to the OS page size.
  this->page_size_ = ::getpagesize();

  // Create two meta pages on a buffer.
  std::string buf;
  buf.reserve(4 * this->page_size_);
  for (int i = 0; i < 2; i++) {
    Page *p = this->page_in_buffer(buf, i);
    p->setID(static_cast<pgid_t>(i));
    p->setFlags(MetaPageFlag);

    // Initialize the meta page.
    Meta *m = p->meta();
    m->magic = Magic;
    m->version = Version;
    m->page_size = static_cast<std::uint32_t>(this->page_size_);
    m->freelist = 2;
    // m->root = new struct bucket();
    m->pgid = 4;
    m->txid = static_cast<txid_t>(i);
    m->checksum = m->sum64();
  }

  // Write an empty freelist at page 3.
  Page *p = this->page_in_buffer(buf, 2);
  p->setID(static_cast<pgid_t>(2));
  p->setFlags(FreelistPageFlag);
  p->setCount(0);

  // Write an empty leaf page at page 4.
  p = this->page_in_buffer(buf, 3);
  p->setID(static_cast<pgid_t>(3));
  p->setFlags(LeafPageFlag);
  p->setCount(0);

  // Write the buffer to our data file.
  this->file_->write_at(buf, 0);
  this->fdatasync();
}

void DB::fdatasync() {
  int r = ::fdatasync(fd());
  if (r != 0) {
    throw std::system_error(errno, std::system_category(), "fdatasync failed");
  }
}

Page *DB::page(pgid_t id) {
  int pos = id * this->pageSize_;
  return reinterpret_cast<Page *>(this->data_ + pos);
}

Tx *DB::begin(bool writable) {
  if (writable) {
    return this->begin_rwtx();
  }
  return this->begin_tx();
}

Tx *DB::begin_tx() {
  // Lock the meta pages while we initialize the transaction.We obtain
  // the meta lock before the mmap lock because that's the order that the
  // write transaction will obtain them.
  this->metalock_.lock();

  // Obtain a read-only lock on the mmap. When the mmap is remapped it will
  // obtain a write lock so all transactions must finish before it can be
  // remapped.
  this->mmaplock_.lock_shared();

  //  Exit if the database is not open yet.
  if (this->opened_) {
    this->mmaplock_.unlock_shared();
    this->metalock_.unlock();
    throw DatabaseNotOpenException();
  }

  // Create a transaction associated with the database.
  Tx *t = new Tx(this);

  // Keep track of transaction until it closes.
  this->txs_.push_back(t);

  // Unlock the meta pages.
  this->metalock_.unlock();

  // Update the transaction stats.
  this->statlock_.lock();
  this->stats_.tx_n++;
  this->stats_.open_tx_n = this->txs_.size();
  this->statlock_.unlock();
  return t;
}

Tx *DB::begin_rwtx() {
  // If the database was opened with Options.ReadOnly, return an error.
  if (this->read_only_) {
    throw DatabaseReadOnlyException();
  }

  // obtain writer lock. This is released by the transaction when it closes.
  // This enforces only one writer transaction at a time.
  this->rwlock_.lock();

  // Once we have the writer lock then we can lock the meta pages so that we can
  // set up the transaction.
  std::lock_guard<std::mutex> metalock(this->metalock_);

  // Exit if the database is not open yet.
  if (this->opened_) {
    this->rwlock_.unlock();
    throw DatabaseNotOpenException();
  }

  // Create a transaction associated with the database.
  Tx *t = new Tx(this, true);
  this->rwtx_ = t;

  // Free any pages associated with closed read-only transactions.
  txid_t minid = 0xFFFFFFFFFFFFFFFF;
  for (auto &tx : this->txs_) {
    if (tx->meta()->txid < minid) {
      minid = tx->meta()->txid;
    }
  }
  if (minid > 0) {
    this->freelist_->release(minid - 1);
  }

  return t;
}
// void removeTx(Tx *);

void DB::update(std::function<void(Tx *)> fn) {
  Tx *tx = this->begin(true);
  fn(tx);
}

DB::~DB() { delete file_; }

int DB::fd() { return file_->fd(); }

Meta *DB::meta() { return new Meta(); }

// flock acquires an advisory lock on a file descriptor
void DB::flock(int timeout) {
  auto start = std::chrono::steady_clock::now();
  for (;;) {
    // If we're beyond our timeout then return an error.
    // This can only occur after we've attempted a flock once.
    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
    if (timeout > 0 && diff.count() > timeout) {
      throw std::runtime_error("flock timeout");
    }
    int flag = !this->read_only_ ? LOCK_EX : LOCK_SH;

    if (::flock(this->fd(), flag) != 0 && errno == EWOULDBLOCK) {
      throw std::runtime_error(std::string("fail to flock:") + std::strerror(errno));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

void DB::close() {
  if (!this->opened_) {
    return;
  }

  this->opened_ = false;
  this->freelist_ = nullptr;

  // clear ops.
  // use file->writeat() directly
  // this->ops.writeAt = nullptr;

  // close the mmap
  this->munmap();

  // close file handles.
  if (this->file_) {
    if (!this->read_only_) {
      try {
        this->funlock();
      } catch (std::exception &e) {
        std::cerr << "bolt.Close(): funlock error: " << e.what() << "\n";
        throw e;
      }
    }

    // Close the file descriptor.
    delete this->file_;
  }

  this->path_ = "";
}

void DB::funlock() {
  if (::flock(this->fd(), LOCK_UN) != 0) {
    char err_info[255];
    sprintf(err_info, "fail to unlock: %s", this->file_->name().c_str());
    throw std::system_error(errno, std::system_category(), err_info);
  }
}

void DB::mmap(int sz) {
  // Map the data file to memory
  void *b = ::mmap(0, sz, PROT_READ, MAP_SHARED | this->mmap_flags_, this->fd(), 0);
  if (b == MAP_FAILED) {
    throw std::system_error(errno, std::system_category(), "mmap failed");
  }

  // Advise the kernel that the mmap is accessed randomly.
  if (::madvise(b, sz, MADV_RANDOM) != 0) {
    throw std::system_error(errno, std::system_category(), "madvise failed");
  }

  // Save the original byte slice and convert to a byte array pointer.
  this->data_ = (char *)b;
  this->data_sz_ = sz;
}

void DB::munmap() {
  // Ignore the unmap if we have no mapped data
  if (!this->data_) {
    return;
  }

  // Unmap using the original byte slice
  int result = ::munmap((void *)this->data_, this->data_sz_);
  this->data_ = nullptr;
  this->data_sz_ = 0;
  if (result != 0) {
    char err_info[255];
    sprintf(err_info, "fail to munmap: %s", this->file_->name().c_str());
    throw std::system_error(errno, std::system_category(), err_info);
  }
}
