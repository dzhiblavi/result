#include "result/combine/lift.h"
#include "result/pipe.h"  // IWYU pragma: keep

#include <gtest/gtest.h>

namespace result {

TEST(Lift, None) {
    auto r = std::optional<int>();
    auto u = r | lift(2);

    static_assert(std::is_same_v<decltype(u), Result<int, int>>);
    EXPECT_TRUE(u.hasError<int>());
    EXPECT_EQ(u.error<int>(), 2);
}

TEST(Lift, Some) {
    auto r = std::optional<int>(1);
    auto u = r | lift(2);

    static_assert(std::is_same_v<decltype(u), Result<int, int>>);
    EXPECT_EQ(*u, 1);
}

}  // namespace result
