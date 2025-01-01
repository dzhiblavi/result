#include "result/combine/and_then.h"
#include "result/pipe.h"

#include <gtest/gtest.h>

namespace result {

template <typename T>
using Res = Result<T, int>;

template <typename T>
Res<T> makeOk(T val) {
    return val;
}

TEST(AndThen, ValueOk) {
    Res<int> r = makeOk(1);
    Res<std::string> u =
        r | andThen([](int val) -> Res<std::string> { return std::to_string(val); });
    EXPECT_TRUE(u);
    EXPECT_EQ(*u, "1");
}

TEST(AndThen, ValueErr) {
    Res<int> r = makeError(1);
    auto u = r | andThen([](int val) -> Res<std::string> { return std::to_string(val); });
    EXPECT_FALSE(u);
    EXPECT_EQ(u.error<int>(), 1);
}

TEST(AndThen, CombineErrorsOkOk) {
    Result<int, int> r = 2;

    Result<float, int, std::string> u =
        r | andThen([](int val) -> Result<float, std::string> { return val * 2.f; });

    EXPECT_TRUE(u);
    EXPECT_EQ(*u, 4.f);
}

TEST(AndThen, CombineErrorsOkErr) {
    Result<int, int> r = 2;

    Result<float, int, std::string> u =
        r |
        andThen([](int) -> Result<float, std::string> { return makeError<std::string>("hello"); });

    EXPECT_TRUE(u.hasError<std::string>());
    EXPECT_EQ(u.error<std::string>(), "hello");
}

TEST(AndThen, CombineErrorsErrOk) {
    Result<int, int> r = makeError(2);

    Result<float, int, std::string> u =
        r |
        andThen([](int) -> Result<float, std::string> { return makeError<std::string>("hello"); });

    EXPECT_TRUE(u.hasError<int>());
    EXPECT_EQ(u.error<int>(), 2);
}

}  // namespace result
