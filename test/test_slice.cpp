#include "bolt/slice.h"
#include <gtest/gtest.h>

TEST(SliceTest, SizeFunc) {
  const char *raw_string = "bolt_cpp";
  Slice s1(raw_string);
  ASSERT_EQ(::strlen(raw_string), s1.size());
  ASSERT_EQ(::memcmp(s1.data(), raw_string, s1.size()), 0);

  std::string str = s1.ToString(true);
  std::cout << str << '\n';

  Slice s2("637070");
  ASSERT_EQ(s2.size(), 6);
  auto[str2, ok] = s2.DecodeHex();
  ASSERT_TRUE(ok);
  ASSERT_EQ(str2, "cpp");
}
