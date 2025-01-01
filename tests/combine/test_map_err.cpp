#include "result/combine/map_err.h"
#include "result/pipe.h"  // IWYU pragma: keep

#include <gtest/gtest.h>

namespace result {

TEST(MapErr, ValueErr) {
    auto r = Result<int, int>(makeError(2));
    auto u = r | mapErr([](int val) { return val * 2L; });
    static_assert(std::is_same_v<decltype(u), Result<int, long>>);
    EXPECT_FALSE(u);
    EXPECT_EQ(u.error<long>(), 4U);
}

TEST(MapErr, ValueOk) {
    auto r = Result<int, int>(2);
    auto u = r | mapErr([](int val) { return val * 2L; });
    static_assert(std::is_same_v<decltype(u), Result<int, long>>);
    EXPECT_EQ(*u, 2);
}

TEST(MapErr, ValueMultiMapErrFirst) {
    auto r = Result<int, int, float>(makeError(2));
    auto u = r | mapErr(detail::Overloaded{
                     [](int val) -> long { return val * 2L; },
                     [](float val) -> double { return val * 3.0; },
                 });
    static_assert(std::is_same_v<decltype(u), Result<int, long, double>>);
    EXPECT_TRUE(u.hasError<long>());
    EXPECT_EQ(u.error<long>(), 4U);
}

TEST(MapErr, ValueMultiMapErrSecond) {
    auto r = Result<int, int, float>(makeError(2.f));
    auto u = r | mapErr(detail::Overloaded{
                     [](int val) -> long { return val * 2L; },
                     [](float val) -> double { return val * 3.0; },
                 });
    static_assert(std::is_same_v<decltype(u), Result<int, long, double>>);
    EXPECT_TRUE(u.hasError<double>());
    EXPECT_EQ(u.error<double>(), 6.);
}

TEST(MapErr, ValueMultiMapOk) {
    auto r = Result<int, int, float>(2);
    auto u = r | mapErr(detail::Overloaded{
                     [](int val) -> long { return val * 2L; },
                     [](float val) -> double { return val * 3.0; },
                 });
    static_assert(std::is_same_v<decltype(u), Result<int, long, double>>);
    EXPECT_EQ(*u, 2);
}

}  // namespace result
