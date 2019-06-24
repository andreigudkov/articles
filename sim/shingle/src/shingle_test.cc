#include <gtest/gtest.h>
#include "isalphanum.h"
#include "shingle.h"

TEST(ShingleTest, IsAlphanum) {
  EXPECT_TRUE(IsAlphanum(48));
  EXPECT_TRUE(IsAlphanum(57));
  EXPECT_TRUE(!IsAlphanum(60));
  EXPECT_TRUE(IsAlphanum(65));
  EXPECT_TRUE(IsAlphanum(5007));
  EXPECT_TRUE(!IsAlphanum(5008));
}


TEST(ShingleTest, Convert) {
  auto pass = [](const char* utf8, uint16_t c) {
    String str;
    Uconv(utf8, str);
    ASSERT_EQ(1, str.length());
    ASSERT_EQ(c, str[0]);
  };
  auto pass2 = [](const char* utf8, const String& expected) {
    String str;
    Uconv(utf8, str);
    ASSERT_EQ(expected, str);
  };
  auto fail = [](const char* utf8) {
    String str;
    EXPECT_THROW(Uconv(utf8, str), std::exception);
  };

  pass("a", 97);
  pass("\xd1\x8f", 1103); // cyrillic small letter "ya"
  pass("\xe2\x89\xa0", 8800); // not equal to
  fail("\xe2\x89"); // not enough bytes
  fail("\xe2\x89\x20"); // invalid aux octet: 00xxxxxx
  fail("\xe2\x89\x60"); // invalid aux octet: 01xxxxxx
  fail("\xe2\x89\xe0"); // invalid aux octet: 11xxxxxx

  // unsupported/illegal octets are replaced with ' '
  pass2("\xf0\x9f\x9d\x8e", String({0x20, 0x20, 0x20, 0x20})); // Alchemical symbol for caput mortuum
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

