#include "bolt/bucket.h"
#include "bolt/node.h"
#include "bolt/page.h"
#include "bolt/tx.h"
#include <cstdlib>
#include <gtest/gtest.h>

void assert_value(Node *n, const char *key, const char *expected_value) {
  std::string value("");
  int retCode = n->get(key, &value);
  ASSERT_EQ(retCode, 0);
  ASSERT_EQ(value, expected_value);
}

TEST(NodeTest, PutKey) {
  Tx tx(nullptr);
  Bucket bucket(&tx);
  Node n(&bucket, true, nullptr);
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

TEST(NodeTest, WriteFunc) {
  Tx tx(nullptr);
  Bucket bucket(&tx);
  Node n(&bucket, true, nullptr);
  n.put("a", "a", "value_a", 1, 0);
  n.put("ab", "ab", "value_ab", 1, 0);
  n.put("abc", "abc", "value_abc", 1, 0);

  char *p1 = (char *)std::malloc(100 * sizeof(char));
  Page p(1, 0, (std::uintptr_t)(p1));
  n.write(&p);
  ASSERT_EQ(p.flags(), LeafPageFlag);
  ASSERT_EQ(p.count(), 3);

  LeafPageElement *elem = p.leafPageElement(0);
  ASSERT_EQ(elem->key(), "a");
  ASSERT_EQ(elem->value(), "value_a");

  elem = p.leafPageElement(1);
  ASSERT_EQ(elem->key(), "ab");
  ASSERT_EQ(elem->value(), "value_ab");

  elem = p.leafPageElement(2);
  ASSERT_EQ(elem->key(), "abc");
  ASSERT_EQ(elem->value(), "value_abc");

  std::free(p1);
}