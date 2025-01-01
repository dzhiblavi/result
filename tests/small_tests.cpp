#include "result/detail/overloaded.h"
#include "result/result.h"

#include "./remember_op.h"

#include <gtest/gtest.h>

namespace result {

struct Nocopy {
    Nocopy() = default;
    Nocopy(const Nocopy&) = delete;
    Nocopy& operator=(const Nocopy&) = delete;
    Nocopy(Nocopy&&) = default;
    Nocopy& operator=(Nocopy&&) = default;

    int value = 1;
};

TEST(DefaultConstruct, Correct) {
    Result<int, float> r;

    EXPECT_TRUE(r.hasValue());
    EXPECT_FALSE(r.hasAnyError());
    EXPECT_FALSE(r.hasError<float>());
    EXPECT_EQ(0, r.value());
}

TEST(DefaultConstruct, NonCopyable) {
    Result<Nocopy, float> r;

    EXPECT_TRUE(r.hasValue());
    EXPECT_FALSE(r.hasAnyError());
    EXPECT_FALSE(r.hasError<float>());
    EXPECT_EQ(1, r.value().value);
    [[maybe_unused]] auto x = std::move(r).value();
}

TEST(MakeError, Correct) {
    Result<int, float, char> r = makeError(1.f);

    EXPECT_FALSE(r.hasValue());
    EXPECT_TRUE(r.hasAnyError());
    EXPECT_TRUE(r.hasError<float>());
    EXPECT_FALSE(r.hasError<char>());
    EXPECT_EQ(1.f, r.error<float>());
}

TEST(MakeError, NonCopyable) {
    Result<int, Nocopy> r = makeError(Nocopy{});

    EXPECT_FALSE(r.hasValue());
    EXPECT_TRUE(r.hasAnyError());
    EXPECT_TRUE(r.hasError<Nocopy>());
    EXPECT_EQ(1, r.error<Nocopy>().value);
    [[maybe_unused]] auto e = std::move(r).error<Nocopy>();
}

TEST(ValueConstruct, Correct) {
    Result<int, int> r = 10;

    EXPECT_TRUE(r.hasValue());
    EXPECT_FALSE(r.hasAnyError());
    EXPECT_FALSE(r.hasError<int>());
    EXPECT_EQ(10, r.value());
}

TEST(CopyConstruct, Value) {
    Result<int, int> r = 10;
    Result<int, char, int> u = r;

    EXPECT_TRUE(u.hasValue());
    EXPECT_EQ(10, u.value());
}

TEST(CopyConstruct, Error) {
    Result<int, float> r = makeError(1.f);
    Result<int, char, float> u = r;

    EXPECT_TRUE(u.hasError<float>());
    EXPECT_EQ(1.f, u.error<float>());
}

TEST(CopyConstruct, ErrorNonCopyable) {
    Result<int, Nocopy> r = makeError(Nocopy{});
    Result<int, char, Nocopy> u = std::move(r);

    EXPECT_TRUE(u.hasError<Nocopy>());
    EXPECT_EQ(1, u.error<Nocopy>().value);
}

TEST(Assign, ValueToValue) {
    Result<int, float> r = 1;
    r = 2;

    EXPECT_TRUE(r.hasValue());
    EXPECT_EQ(2, r.value());
}

TEST(Assign, ValueToError) {
    Result<int, float> r = makeError(1.f);
    r = 2;

    EXPECT_TRUE(r.hasValue());
    EXPECT_EQ(2, r.value());
}

TEST(Assign, ErrorToValue) {
    Result<int, float> r = 1;
    r = makeError(1.f);

    EXPECT_TRUE(r.hasError<float>());
    EXPECT_EQ(1.f, r.error<float>());
}

TEST(Assign, ErrorToError) {
    Result<int, float> r = makeError(2.f);
    r = makeError(1.f);

    EXPECT_TRUE(r.hasError<float>());
    EXPECT_EQ(1.f, r.error<float>());
}

TEST(Assign, ValueToErrorNonCopyable) {
    Result<int, Nocopy> r = makeError(Nocopy{});
    r = 1;

    EXPECT_TRUE(r.hasValue());
    EXPECT_EQ(1, r.value());
}

TEST(Assign, ErrorToValueNonCopyable) {
    Result<Nocopy, int> r = Nocopy{};
    r = makeError(1);

    EXPECT_TRUE(r.hasError<int>());
    EXPECT_EQ(1, r.error<int>());
}

TEST(Convertible, Concept) {
    static_assert(ConvertibleTo<Result<int, float>, Result<int, float, char>>);
    static_assert(!ConvertibleTo<Result<int, float, char>, Result<int, float>>);
    static_assert(ConvertibleTo<Result<detail::Impossible, float>, Result<int, float>>);
    static_assert(!ConvertibleTo<Result<char, float>, Result<int, float>>);
}

TEST(ConvertConstruct, SameErrIndex) {
    Result<int, float, short> r = makeError(1.f);
    Result<int, float, char, double, short> u = r;

    EXPECT_TRUE(u.hasError<float>());
    EXPECT_EQ(1.f, u.error<float>());
}

TEST(ConvertConstruct, DifferentErrIndex) {
    Result<int, float, int> r = makeError(1.f);
    Result<int, char, float, double, int> u = r;

    EXPECT_TRUE(u.hasError<float>());
    EXPECT_EQ(1.f, u.error<float>());
}

TEST(SwitchIndex, Correct) {
    Result<int, float, int> r = makeError(1.f);

    switch (r.index()) {
        case r.valueIndex():
            FAIL();
            break;
        case r.errorIndex<float>():
            EXPECT_EQ(1.f, r.error<float>());
            break;
        case r.errorIndex<int>():
            FAIL();
            break;
        default:
            FAIL();
    }
}

TEST(Visit, Return) {
    Result<int, float, int> r = makeError(2);

    int x = r.visit(detail::Overloaded{
        [](int x) { return x * 2; },
    });

    EXPECT_EQ(x, 4);
}

}  // namespace result
