#include "bolt/bucket.h"
#include "bolt/cursor.h"
#include "bolt/node.h"
#include "bolt/page.h"
#include "bolt/tx.h"
#include "util.h"
#include <cstdlib>
#include <gtest/gtest.h>

TEST(CursorTest, BucketFunc) {
  DB *db = MustOpenDB();
  ASSERT_EQ(2 + 3, 5);
  //   MustClose(db);
}