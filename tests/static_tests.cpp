#include "result/tools.h"
#include "result/value_or_error.h"

#include <gtest/gtest.h>
#include <limits>
#include <type_traits>

namespace result::detail {

namespace list = ::util::list;

TEST(AllDecayedTest, Correctness) {
    static_assert(AllDecayed<>);
    static_assert(AllDecayed<int, float*, void>);
    static_assert(!AllDecayed<int&>);
    static_assert(!AllDecayed<int[]>);
    static_assert(!AllDecayed<int, float[], std::string, int*>);
    static_assert(!AllDecayed<int, float[], std::string&, int*>);
}

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

TEST(IndexMappingTest, Correctness) {
    {
        using M = IndexMapping<>::MapTo<>;
        static_assert(0 == sizeof(M::indices_));
    }

    {
        using M = IndexMapping<int, float, char, std::string>::MapTo<int, float, char, std::string>;
        static_assert(0 == M::indices_[0]);
        static_assert(1 == M::indices_[1]);
        static_assert(2 == M::indices_[2]);
        static_assert(3 == M::indices_[3]);
    }

    {
        using M = IndexMapping<int, float, char, std::string>::MapTo<float, char, std::string, int>;
        static_assert(3 == M::indices_[0]);
        static_assert(0 == M::indices_[1]);
        static_assert(1 == M::indices_[2]);
        static_assert(2 == M::indices_[3]);
    }

    {
        using M = IndexMapping<int>::MapTo<ValueTypeWrapper<int>, int>;
        static_assert(1 == M::indices_[0]);
    }

    {
        using M = IndexMapping<int, float, short>::MapTo<float, char, int>;
        static_assert(2 == M::indices_[0]);
        static_assert(0 == M::indices_[1]);
        static_assert(size_t(-1) == M::indices_[2]);
    }
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

TEST(ConvertibleTest, Correctness) {
    static_assert(Convertible<list::list<void>, list::list<void>>);
    static_assert(Convertible<list::list<int>, list::list<void>>);
    static_assert(Convertible<list::list<void>, list::list<int>>);
    static_assert(Convertible<list::list<int>, list::list<int>>);
    static_assert(!Convertible<list::list<int>, list::list<float>>);
    static_assert(Convertible<list::list<int, int>, list::list<int, int, short>>);
    static_assert(Convertible<list::list<int, int>, list::list<void, int, short>>);
    static_assert(Convertible<list::list<int, int>, list::list<void, short, float, int>>);
    static_assert(!Convertible<list::list<int, int>, list::list<float, int, short>>);
    static_assert(Convertible<list::list<int, char>, list::list<int, short, char>>);
    static_assert(Convertible<list::list<void, const char*>, list::list<int, const char*>>);
    static_assert(!Convertible<list::list<int, char>, list::list<int, short>>);
    static_assert(!Convertible<list::list<void, char>, list::list<int, short>>);
    static_assert(Convertible<list::list<void, char, int>, list::list<int, short, char, int>>);
}

TEST(VariantStorageTest, Alignment) {
    static_assert(alignof(Result<char>) == alignof(char));
    static_assert(alignof(Result<char, short>) == alignof(short));
    static_assert(alignof(Result<short, char>) == alignof(short));
    static_assert(alignof(Result<short, char, void*>) == alignof(void*));
    static_assert(alignof(Result<void*, short, char>) == alignof(void*));
    static_assert(alignof(Result<void, char>) == alignof(char));
    static_assert(alignof(Result<void, char, short>) == alignof(short));
    static_assert(alignof(Result<void, short, char>) == alignof(short));
    static_assert(alignof(Result<void, short, char, void*>) == alignof(void*));
    static_assert(alignof(Result<void, void*, short, char>) == alignof(void*));
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

TEST(Result, TriviallyDestructible) {
    static_assert(std::is_trivially_destructible_v<Result<void>>);
    static_assert(std::is_trivially_destructible_v<Result<int>>);
    static_assert(std::is_trivially_destructible_v<Result<int, float, char>>);
    static_assert(!std::is_trivially_destructible_v<Result<int, std::string>>);
    static_assert(!std::is_trivially_destructible_v<Result<std::vector<int>, float>>);
    static_assert(std::is_trivially_destructible_v<Result<int, char>>);
    static_assert(!std::is_trivially_destructible_v<Result<int, std::unique_ptr<int>>>);
}

TEST(ValueConstructorTest, Nothrow) {
    static_assert(std::is_nothrow_constructible_v<Result<int>, int>);
    static_assert(!std::is_nothrow_constructible_v<Result<std::string>, std::string>);
}

Result<int, const char*> ReturnValue() {
    return 42;
}

Result<int, const char*> ReturnError() {
    return MakeError<const char*>("an error");
}

TEST(GetValueTest, RefQualifiers) {
    {
        Result<int> verr(42);
        static_assert(std::is_same_v<decltype(std::move(verr).GetValue()), int&&>);
        static_assert(std::is_same_v<decltype(verr.GetValue()), int&>);
        static_assert(std::is_same_v<decltype(ReturnValue().GetValue()), int&&>);
        static_assert(
            std::is_same_v<decltype(static_cast<const Result<int>&>(verr).GetValue()), const int&>);
    }
    {
        Result<bool, int> verr = MakeError<int>(42);
        static_assert(std::is_same_v<decltype(std::move(verr).GetError<int>()), int&&>);
        static_assert(std::is_same_v<decltype(verr.GetError<int>()), int&>);
        static_assert(std::is_same_v<
                      decltype(static_cast<const Result<bool, int>&>(verr).GetError<int>()),
                      const int&>);
        static_assert(
            std::is_same_v<decltype(ReturnError().GetError<const char*>()), const char*&&>);
    }
}

TEST(InvertTest, ReturnType) {
    static_assert(std::is_same_v<
                  Result<std::tuple<char>, float, int, short>,
                  InvertResultType<Result<void, int, float>, Result<char, int, short>>>);
}

}  // namespace result::detail
