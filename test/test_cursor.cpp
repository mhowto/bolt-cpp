#include "bolt/bucket.h"
#include "bolt/cursor.h"
#include "bolt/node.h"
#include "bolt/page.h"
#include "bolt/tx.h"
#include "util.h"
#include <cstdlib>
#include <gtest/gtest.h>

TEST(CursorTest, BucketFunc) {
  std::cout << "TO test mustopen" << std::endl;
  DB *db = MustOpenDB();
  ASSERT_TRUE(db != nullptr);
}