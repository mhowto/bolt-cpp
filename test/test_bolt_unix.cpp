#include <gtest/gtest.h>
#include "bolt/db.h"
#include "bolt/bolt_unix.h"

TEST(MmapDBTest, MmapFunc) {
    DB db("haha.db", 0, 0);
    mmap(&db, 10000);
}
