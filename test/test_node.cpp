#include "bolt/bucket.h"
#include "bolt/node.h"
#include "bolt/tx.h"
#include <gtest/gtest.h>

void assert_value(Node *n, const char *key, const char *expected_value) {
  std::string value("");
  int retCode = n->get(key, &value);
  ASSERT_EQ(retCode, 0);
  ASSERT_EQ(value, expected_value);
}

TEST(NodeTest, PutKey) {
  //   DB db();
  Tx tx(nullptr);
  Bucket bucket(&tx);
  Node n(&bucket, nullptr);
  n.put("aa", "aa", "value_aa", 1, 0);
  n.put("ab", "ab", "value_ab", 1, 0);
  ASSERT_EQ(n.numChildren(), 2);
  assert_value(&n, "aa", "value_aa");

  n.put("aa", "aa", "value_aaa", 1, 0);
  ASSERT_EQ(n.numChildren(), 2);
  assert_value(&n, "aa", "value_aaa");

  n.put("ac", "ac", "value_ac", 1, 0);
  ASSERT_EQ(n.numChildren(), 3);
  assert_value(&n, "ab", "value_ab");
  assert_value(&n, "ac", "value_ac");

  n.del("ab");
  ASSERT_EQ(n.numChildren(), 2);
  n.del("aa");
  ASSERT_EQ(n.numChildren(), 1);
  n.del("ac");
  ASSERT_EQ(n.numChildren(), 0);
}

TEST(NodeTest, PageElementSizeFunc) {}