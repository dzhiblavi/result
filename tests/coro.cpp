#include "result/coro.h"

#include <gtest/gtest.h>

namespace result {

template <typename T>
using Res = Result<T, std::string>;

Res<int> just(int x) {
    co_return x;
}

Res<int> faulty(std::string err) {
    co_return makeError(err);
}

TEST(Coro, CoJust) {
    auto x = just(1);
    EXPECT_EQ(x.getValue(), 1);
}

TEST(Coro, CoJustLinear) {
    Res<int> x = [] -> Res<int> {
        int x = co_await just(1);
        int y = co_await just(2);
        co_return x + y;
    }();

    EXPECT_TRUE(x.hasValue());
    EXPECT_EQ(x.getValue(), 3);
}

TEST(Coro, Faulty) {
    Res<int> x = [] -> Res<int> {
        int x = co_await just(1);
        int y = co_await faulty("hello");
        co_return x + y;
    }();

    EXPECT_FALSE(x.hasValue());
    EXPECT_TRUE(x.hasAnyError());
    EXPECT_EQ(x.getError<std::string>(), "hello");
}

}  // namespace result
