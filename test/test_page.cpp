#include "bolt/page.h"
#include <gtest/gtest.h>

TEST(PageTest, TypeFunc) {
  Page p(0, PageFlag::BranchPageFlag, 0);
  ASSERT_EQ(p.type(), "branch");

  p = Page(0, PageFlag::LeafPageFlag, 0);
  ASSERT_EQ(p.type(), "leaf");

  p = Page(0, PageFlag::MetaPageFlag, 0);
  ASSERT_EQ(p.type(), "meta");

  p = Page(0, 20000, 0);
  ASSERT_EQ(p.type(), "unknown<20000>");
}

TEST(PageTest, DumpFunc) {
  Page p(256, PageFlag::BranchPageFlag, 0);
  p.hexdump(16);
}

TEST(PageTest, ConstSize) {
  ASSERT_EQ(pageHeaderSize, 24);
  ASSERT_EQ(leafPageElementSize, 16);
  ASSERT_EQ(branchPageElementSize, 16);
}