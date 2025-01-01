#include "result/combine/or_else.h"
#include "result/pipe.h"  // IWYU pragma: keep

#include <gtest/gtest.h>

namespace result {

TEST(OrElse, ValueErr) {
    auto r = Result<int, int>(makeError(2));
    auto u = r | orElse([](int err) -> Result<int, int> { return err * 2; });
    static_assert(std::is_same_v<decltype(u), Result<int, int>>);
    EXPECT_EQ(*u, 4);
}

TEST(OrElse, ValueOk) {
    auto r = Result<int, int>(2);
    auto u = r | orElse([](int err) -> Result<int, int> { return err * 2; });
    static_assert(std::is_same_v<decltype(u), Result<int, int>>);
    EXPECT_EQ(*u, 2);
}

TEST(OrElse, CombineErrorsLessOk) {
    auto r = Result<int, int, float>(1);
    auto u = r | orElse(detail::Overloaded{
                     [](int err) -> Result<int, int> { return err * 2; },
                     [](float err) -> Result<int, int> { return err * 2; },
                 });

    static_assert(std::is_same_v<decltype(u), Result<int, int>>);
    EXPECT_EQ(*u, 1);
}

TEST(OrElse, CombineErrorsMoreErr1) {
    auto r = Result<int, int, float>(makeError(1));
    auto u = r | orElse(detail::Overloaded{
                     [](int) -> Result<int, long, double> { return makeError(5L); },
                     [](float) -> Result<int, double, short> { return makeError(8.0); },
                 });

    static_assert(std::is_same_v<decltype(u), Result<int, long, double, short>>);
    EXPECT_TRUE(u.hasError<long>());
    EXPECT_EQ(u.error<long>(), 5L);
}

TEST(OrElse, CombineErrorsMoreErr2) {
    auto r = Result<int, int, float>(makeError(1.f));
    auto u = r | orElse(detail::Overloaded{
                     [](int) -> Result<int, long, double> { return makeError(5L); },
                     [](float) -> Result<int, double, short> { return makeError(8.0); },
                 });

    static_assert(std::is_same_v<decltype(u), Result<int, long, double, short>>);
    EXPECT_TRUE(u.hasError<double>());
    EXPECT_EQ(u.error<double>(), 8.0);
}

}  // namespace result
