#include "bolt_unix.h"
#include "db.h"

#include <sys/mman.h>
#include <sys/file.h>
#include <stdexcept>
#include <cstring>
#include <thread>
#include <chrono>

void mmap(DB* db, int sz) {
    // Map the data file to memory
    void* b = ::mmap(0, sz, PROT_READ, MAP_SHARED | db->MapFlags, db->FileDescriptor, 0);
    if (b == MAP_FAILED) {
        char err_info[255];
        sprintf(err_info, "mmap failed:%s", std::strerror(errno));
        throw std::runtime_error(err_info);
    }

    // Advise the kernel that the mmap is accessed randomly.
    if (::madvise(b, sz, MADV_RANDOM) != 0) {
        char err_info[255];
        sprintf(err_info, "madvise failed:%s", std::strerror(errno));
        throw std::runtime_error(err_info);
    }

    // Save the original byte slice and convert to a byte array pointer.
    db->Data = (byte*)b;
    db->DataSize = sz;
}

// munmap unmaps a DB's data file from memory
void munmap(DB* db) {
    // Ignore the unmap if we have no mapped data
    if (db->Data == nullptr) {
        return;
    }

    // Unmap using the original byte slice
    int result = ::munmap((void*) db->Data, db->DataSize);
    db->Data = nullptr;
    db->DataSize = 0;
    if (result != 0) {
         throw std::runtime_error(std::string("fail to munmap:") + std::strerror(errno));
    }
}

// flock acquires an advisory lock on a file descriptor
void flock(DB* db, FileMode mode, bool exclusive, int timeout) {
    auto start = std::chrono::steady_clock::now();
    for(;;) {
        // If we're beyond our timeout then return an error.
        // This can only occur after we've attempted a flock once.
        auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        if (timeout > 0 && diff.count() > timeout) {
            throw std::runtime_error("flock timeout");
        }
        int flag = exclusive ? LOCK_EX : LOCK_SH;

        if (::flock(db->FileDescriptor, flag) != 0 && errno == EWOULDBLOCK) {
            throw std::runtime_error(std::string("fail to flock:") + std::strerror(errno));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void funlock(DB* db) {
    if (::flock(db->FileDescriptor, LOCK_UN) != 0) {
        throw std::runtime_error(std::string("fail to unlock:") + std::strerror(errno));
    }
}
