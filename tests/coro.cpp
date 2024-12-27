#include "voe/coro.h"

#include <gtest/gtest.h>

namespace voe {

template <typename T>
using Res = ValueOrError<T, std::string>;

Res<int> just(int x) {
    co_return x;
}

Res<int> faulty(std::string err) {
    co_return MakeError(err);
}

TEST(Coro, CoJust) {
    auto x = just(1);
    EXPECT_EQ(x.GetValue(), 1);
}

TEST(Coro, CoJustLinear) {
    Res<int> x = [] -> Res<int> {
        int x = co_await just(1);
        int y = co_await just(2);
        co_return x + y;
    }();

    EXPECT_TRUE(x.HasValue());
    EXPECT_EQ(x.GetValue(), 3);
}

TEST(Coro, Faulty) {
    Res<int> x = [] -> Res<int> {
        int x = co_await just(1);
        int y = co_await faulty("hello");
        co_return x + y;
    }();

    EXPECT_FALSE(x.HasValue());
    EXPECT_TRUE(x.HasAnyError());
    EXPECT_EQ(x.GetError<std::string>(), "hello");
}

}  // namespace voe
