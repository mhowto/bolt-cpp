#include "bolt/bucket.h"
#include "bolt/node.h"
#include "bolt/tx.h"
#include <gtest/gtest.h>

TEST(NodeTest, PutKey) {
  //   DB db();
  Tx tx(nullptr);
  Bucket bucket(&tx);
  Node n(&bucket, nullptr);
  n.put("aa", "aa", "value_aa", 1, 0);
  n.put("ab", "ab", "value_ab", 1, 0);
  ASSERT_EQ(n.numChildren(), 2);
  n.put("aa", "aa", "value_aaa", 1, 0);
  ASSERT_EQ(n.numChildren(), 2);
  n.put("ac", "ac", "value_ac", 1, 0);
  ASSERT_EQ(n.numChildren(), 3);
}

TEST(NodeTest, PageElementSizeFunc) {}