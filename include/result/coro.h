#pragma once

#include "result/detail/coro/awaitable.h"
#include "result/detail/coro/promise.h"

template <typename T, typename... Es>
auto operator co_await(result::Result<T, Es...> o) {
    return ::result::detail::ResultAwaitable<T, Es...>{std::move(o)};
}

namespace std {

template <typename T, typename... Es, typename... Args>
struct coroutine_traits<::result::Result<T, Es...>, Args...> {
    using promise_type = ::result::detail::ResultPromise<T, Es...>;
};

}  // namespace std
