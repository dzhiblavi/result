#include "result/combine/map.h"
#include "result/pipe.h"  // IWYU pragma: keep

#include <gtest/gtest.h>

namespace result {

TEST(Map, ValueOk) {
    auto r = Result<int, int>(2);
    auto u = r | map([](int val) { return std::to_string(val); });
    static_assert(std::is_same_v<decltype(u), Result<std::string, int>>);
    EXPECT_EQ(*u, "2");
}

TEST(Map, ValueErr) {
    auto r = Result<int, int>(makeError(2));
    auto u = r | map([](int val) { return std::to_string(val); });
    static_assert(std::is_same_v<decltype(u), Result<std::string, int>>);
    EXPECT_FALSE(u);
    EXPECT_EQ(u.error<int>(), 2);
}

}  // namespace result
