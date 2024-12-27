#include "voe/tools.h"
#include "voe/value_or_error.h"

#include <gtest/gtest.h>

namespace voe {

TEST(SmallDefaultConstructorTest, Correctness) {
  {
    ValueOrError<void> voe;
    EXPECT_TRUE(voe.IsEmpty());
    EXPECT_FALSE(voe.HasAnyError());
    EXPECT_FALSE(voe.HasValue());
  }
  {
    ValueOrError<int, char> voe;
    EXPECT_TRUE(voe.IsEmpty());
    EXPECT_FALSE(voe.HasAnyError());
    EXPECT_FALSE(voe.HasValue());
    EXPECT_DEATH(voe.GetValue(), "");
  }
}

TEST(SmallValueConstructorTest, Correctness) {
  ValueOrError<int> voe{10};
  EXPECT_FALSE(voe.IsEmpty());
  EXPECT_FALSE(voe.HasAnyError());
  EXPECT_TRUE(voe.HasValue());
  EXPECT_EQ(10, voe.GetValue());
}

TEST(SmallConvertConstructorTest, FromEmpty) {
  ValueOrError<int, char> from;
  ValueOrError<int, short, char> voe(from);
  EXPECT_TRUE(voe.IsEmpty());
  EXPECT_FALSE(voe.HasAnyError());
  EXPECT_FALSE(voe.HasValue());
}

TEST(SmallConvertConstructorTest, FromValue) {
  ValueOrError<int, char> from{10};
  ValueOrError<int, short, char> voe(from);
  EXPECT_FALSE(voe.IsEmpty());
  EXPECT_FALSE(voe.HasAnyError());
  EXPECT_TRUE(voe.HasValue());
  EXPECT_EQ(10, voe.GetValue());
}

TEST(SmallConvertConstructorTest, FromError) {
  ValueOrError<int, char> from{MakeError<char>('a')};
  ValueOrError<int, short, char> voe(from);
  EXPECT_FALSE(voe.IsEmpty());
  EXPECT_TRUE(voe.HasAnyError());
  EXPECT_FALSE(voe.HasError<short>());
  EXPECT_TRUE(voe.HasError<char>());
  EXPECT_FALSE(voe.HasValue());
  EXPECT_EQ('a', voe.GetError<char>());
}

TEST(SmallConvertConstructorDeathTest, ValueIsDropped) {
  ValueOrError<int, char> from{10};
  EXPECT_DEATH(((void)ValueOrError<void, char>(from)), "trying to drop a value");
}

TEST(SmallConvertAssignTest, FromEmpty) {
  {
    ValueOrError<int, short, char> to;
    ValueOrError<int, char> from;
    to = from;
    EXPECT_TRUE(to.IsEmpty());
    EXPECT_FALSE(to.HasAnyError());
    EXPECT_FALSE(to.HasValue());
  }
  {
    ValueOrError<int, short, char> to{10};
    ValueOrError<int, char> from;
    to = from;
    EXPECT_TRUE(to.IsEmpty());
    EXPECT_FALSE(to.HasAnyError());
    EXPECT_FALSE(to.HasValue());
  }
  {
    ValueOrError<int, short, char> to{MakeError<short>(8)};
    ValueOrError<int, char> from;
    to = from;
    EXPECT_TRUE(to.IsEmpty());
    EXPECT_FALSE(to.HasAnyError());
    EXPECT_FALSE(to.HasValue());
  }
}

TEST(SmallConvertAssignTest, FromValue) {
  {
    ValueOrError<int, short, char> to;
    ValueOrError<int, char> from{10};
    to = from;
    EXPECT_TRUE(to.HasValue());
    EXPECT_EQ(10, to.GetValue());
  }
  {
    ValueOrError<int, short, char> to{-239};
    ValueOrError<int, char> from{10};
    to = from;
    EXPECT_TRUE(to.HasValue());
    EXPECT_EQ(10, to.GetValue());
  }
  {
    ValueOrError<int, short, char> to{MakeError<short>(8)};
    ValueOrError<int, char> from{10};
    to = from;
    EXPECT_TRUE(to.HasValue());
    EXPECT_EQ(10, to.GetValue());
  }
}

TEST(SmallConvertAssignTest, FromError) {
  {
    ValueOrError<int, short, char> to;
    ValueOrError<int, char> from{MakeError<char>('a')};
    to = from;
    EXPECT_TRUE(to.HasError<char>());
    EXPECT_EQ('a', to.GetError<char>());
  }
  {
    ValueOrError<int, short, char> to{-239};
    ValueOrError<int, char> from{MakeError<char>('a')};
    to = from;
    EXPECT_TRUE(to.HasError<char>());
    EXPECT_EQ('a', to.GetError<char>());
  }
  {
    ValueOrError<int, short, char> to{MakeError<short>(8)};
    ValueOrError<int, char> from{MakeError<char>('a')};
    to = from;
    EXPECT_TRUE(to.HasError<char>());
    EXPECT_EQ('a', to.GetError<char>());
  }
}

ValueOrError<int, const char*> ReturnValue() {
  return 42;
}

TEST(ValueOrErrorTest, ValueOnReturn) {
  auto val = ReturnValue();
  EXPECT_FALSE(val.IsEmpty());
  EXPECT_FALSE(val.HasAnyError());
  EXPECT_FALSE(val.HasError<const char*>());
  EXPECT_TRUE(val.HasValue());
  EXPECT_EQ(val.GetValue(), 42);
  EXPECT_EQ(std::move(val).GetValue(), 42);
  EXPECT_EQ(ReturnValue().GetValue(), 42);

  const auto cval = ReturnValue();
  EXPECT_FALSE(cval.IsEmpty());
  EXPECT_FALSE(cval.HasAnyError());
  EXPECT_FALSE(cval.HasError<const char*>());
  EXPECT_TRUE(cval.HasValue());
  EXPECT_EQ(cval.GetValue(), 42);
}

ValueOrError<int, const char*> ReturnEmpty() {
  ValueOrError<int, const char*> val;
  return val;
}

TEST(ValueOrErrorTest, ReturnEmpty) {
  auto val = ReturnEmpty();
  EXPECT_TRUE(val.IsEmpty());
  EXPECT_FALSE(val.HasAnyError());
  EXPECT_FALSE(val.HasError<const char*>());
  EXPECT_FALSE(val.HasValue());
}

ValueOrError<int, const char*> ReturnError() {
  return MakeError<const char*>("some error");
}

TEST(ValueOrErrorTest, ReturnError) {
  auto val = ReturnError();
  EXPECT_FALSE(val.IsEmpty());
  EXPECT_TRUE(val.HasAnyError());
  EXPECT_TRUE(val.HasError<const char*>());
  EXPECT_FALSE(val.HasValue());
  EXPECT_STREQ(val.GetError<const char*>(), "some error");
  EXPECT_STREQ(std::move(val).GetError<const char*>(), "some error");
  EXPECT_STREQ(ReturnError().GetError<const char*>(), "some error");

  const auto cval = ReturnError();
  EXPECT_FALSE(cval.IsEmpty());
  EXPECT_TRUE(cval.HasAnyError());
  EXPECT_TRUE(cval.HasError<const char*>());
  EXPECT_FALSE(cval.HasValue());
  EXPECT_STREQ(cval.GetError<const char*>(), "some error");
}

ValueOrError<int, bool, const char*> CallReturnError() {
  return ReturnError();
}

TEST(ValueOrErrorTest, Conversion) {
  auto val = CallReturnError();
  EXPECT_FALSE(val.IsEmpty());
  EXPECT_TRUE(val.HasAnyError());
  EXPECT_TRUE(val.HasError<const char*>());
  EXPECT_FALSE(val.HasValue());
  EXPECT_EQ(val.GetErrorIndex(), 1U);
  EXPECT_STREQ(val.GetError<const char*>(), "some error");
}

ValueOrError<int, const char*> DiscardBoolError() {
  auto val = CallReturnError();
  EXPECT_EQ(val.GetErrorIndex(), 1U);
  return val.DiscardErrors<bool>();
}

TEST(ValueOrErrorTest, DiscardErrorType) {
  auto val = DiscardBoolError();
  EXPECT_FALSE(val.IsEmpty());
  EXPECT_TRUE(val.HasAnyError());
  EXPECT_TRUE(val.HasError<const char*>());
  EXPECT_FALSE(val.HasValue());
  EXPECT_STREQ(val.GetError<const char*>(), "some error");
}

ValueOrError<int, const char*, bool> ErrorPermutation() {
  return CallReturnError();
}

TEST(ValueOrErrorTest, ErrorPermutation) {
  auto val = ErrorPermutation();
  EXPECT_FALSE(val.IsEmpty());
  EXPECT_TRUE(val.HasAnyError());
  EXPECT_TRUE(val.HasError<const char*>());
  EXPECT_FALSE(val.HasValue());
  EXPECT_EQ(val.GetErrorIndex(), 0U);
  EXPECT_STREQ(val.GetError<const char*>(), "some error");
}

TEST(ValueOrErrorTest, Invert) {
  {
    ValueOrError<void, int, float> v1 = {};
    ValueOrError<char, float, short> v2 = 'a';

    auto inverted = invert(v1, v2);
    EXPECT_FALSE(inverted.HasAnyError());
    EXPECT_TRUE(inverted.HasValue());
    EXPECT_EQ(std::make_tuple('a'), inverted.GetValue());
  }

  {
    ValueOrError<int, int, float> v1 = 10;
    ValueOrError<char, float, short> v2 = 'a';
    ValueOrError<std::string, float, short> v3 = std::string("Hello");

    auto inverted = invert(v1, v2, v3);
    EXPECT_FALSE(inverted.HasAnyError());
    EXPECT_TRUE(inverted.HasValue());
    EXPECT_EQ(std::make_tuple(10, 'a', std::string("Hello")), inverted.GetValue());
  }

  {
    ValueOrError<void, int, float> v1 = voe::MakeError(10);
    ValueOrError<char, float, short> v2 = 'a';

    auto inverted = invert(v1, v2);
    EXPECT_TRUE(inverted.HasAnyError());
    EXPECT_TRUE(inverted.HasError<int>());
    EXPECT_FALSE(inverted.HasValue());
    EXPECT_EQ(10, inverted.GetError<int>());
  }

  {
    ValueOrError<int, int, float> v1 = 10;
    ValueOrError<char, float, short> v2 = MakeError(10.f);
    ValueOrError<std::string, float, short> v3 = std::string("Hello");

    auto inverted = invert(v1, v2, v3);
    EXPECT_TRUE(inverted.HasAnyError());
    EXPECT_TRUE(inverted.HasError<float>());
    EXPECT_FALSE(inverted.HasValue());
    EXPECT_EQ(10.f, inverted.GetError<float>());
  }

  {
    ValueOrError<int, int, float> v1 = 10;
    ValueOrError<char, float, short> v2 = 'a';
    ValueOrError<std::string, float, short> v3 = MakeError<short>(8);

    auto inverted = invert(v1, v2, v3);
    EXPECT_TRUE(inverted.HasAnyError());
    EXPECT_TRUE(inverted.HasError<short>());
    EXPECT_FALSE(inverted.HasValue());
    EXPECT_EQ(8, inverted.GetError<short>());
  }
}

}  // namespace voe
