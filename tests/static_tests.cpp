#include "result/result.h"
#include "result/union.h"

#include <gtest/gtest.h>

#include <limits>
#include <type_traits>

namespace result::detail {

TEST(VoEUnionTest, Correctness) {
    static_assert(std::is_same_v<Union<void>, Result<void>>);
    static_assert(std::is_same_v<Union<void, int, char>, Result<void, int, char>>);
    static_assert(std::is_same_v<Union<void, Result<void>, Result<void>>, Result<void>>);
    static_assert(std::is_same_v<
                  Union<void, Result<char, int>, Result<float, std::string>>,
                  Result<void, int, std::string>>);
    static_assert(std::is_same_v<
                  Union<
                      void,
                      Result<char, int, char>,
                      Result<short>,
                      Result<float, std::string, int, short>,
                      Result<float, short>>,
                  Result<void, char, std::string, int, short>>);
    static_assert(std::is_same_v<
                  Union<
                      void,
                      Result<char, int, char>,
                      int,
                      short,
                      Result<short>,
                      Result<float, std::string, int, short>,
                      std::string,
                      int64_t,
                      Result<float, short>>,
                  Result<void, char, int, std::string, int64_t, short>>);
}

TEST(MinimalSizedIndexTypeTest, Correctness) {
    static_assert(1 == sizeof(MinimalSizedIndexType<0>));
    static_assert(1 == sizeof(MinimalSizedIndexType<123>));
    static_assert(1 == sizeof(MinimalSizedIndexType<std::numeric_limits<uint8_t>::max()>));
    static_assert(2 == sizeof(MinimalSizedIndexType<1ull << 8>));
    static_assert(2 == sizeof(MinimalSizedIndexType<std::numeric_limits<uint16_t>::max()>));
    static_assert(4 == sizeof(MinimalSizedIndexType<1ull << 16>));
    static_assert(4 == sizeof(MinimalSizedIndexType<std::numeric_limits<uint32_t>::max()>));
    static_assert(8 == sizeof(MinimalSizedIndexType<1ull << 32>));
    static_assert(8 == sizeof(MinimalSizedIndexType<std::numeric_limits<uint64_t>::max()>));
}

TEST(VariantStorageTest, Alignment) {
    static_assert(alignof(Result<char>) == alignof(char));
    static_assert(alignof(Result<char, short>) == alignof(short));
    static_assert(alignof(Result<short, char>) == alignof(short));
    static_assert(alignof(Result<short, char, void*>) == alignof(void*));
    static_assert(alignof(Result<void*, short, char>) == alignof(void*));
    static_assert(alignof(Result<std::monostate, char>) == alignof(char));
    static_assert(alignof(Result<std::monostate, char, short>) == alignof(short));
    static_assert(alignof(Result<std::monostate, short, char>) == alignof(short));
    static_assert(alignof(Result<std::monostate, short, char, void*>) == alignof(void*));
    static_assert(alignof(Result<std::monostate, void*, short, char>) == alignof(void*));
}

TEST(VariantStorageTest, Size) {
    struct Chars {
        char data[7];
    };
    static_assert(sizeof(Result<int, char*>) == 16);
    static_assert(sizeof(Result<int, bool, char, short>) == 8);
    static_assert(sizeof(Result<int, Chars>) == 8);
    static_assert(sizeof(Result<int, char*>) == 16);
    static_assert(sizeof(Result<int, bool, char, short>) == 8);
}

Result<int, const char*> ReturnValue() {
    return 42;
}

Result<int, const char*> ReturnError() {
    return makeError<const char*>("an error");
}

TEST(GetValueTest, RefQualifiers) {
    {
        Result<int> verr(42);
        static_assert(std::is_same_v<decltype(std::move(verr).getValue()), int&&>);
        static_assert(std::is_same_v<decltype(verr.getValue()), int&>);
        static_assert(std::is_same_v<decltype(ReturnValue().getValue()), int&&>);
        static_assert(
            std::is_same_v<decltype(static_cast<const Result<int>&>(verr).getValue()), const int&>);
    }
    {
        Result<bool, int> verr = makeError<int>(42);
        static_assert(std::is_same_v<decltype(std::move(verr).getError<int>()), int&&>);
        static_assert(std::is_same_v<decltype(verr.getError<int>()), int&>);
        static_assert(std::is_same_v<
                      decltype(static_cast<const Result<bool, int>&>(verr).getError<int>()),
                      const int&>);
        static_assert(
            std::is_same_v<decltype(ReturnError().getError<const char*>()), const char*&&>);
    }
}

}  // namespace result::detail
