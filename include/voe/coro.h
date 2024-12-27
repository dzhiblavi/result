#pragma once

#include "voe/detail/coro/awaitable.h"
#include "voe/detail/coro/promise.h"

template <typename T, typename... Es>
auto operator co_await(voe::ValueOrError<T, Es...> o) {
    return ::voe::detail::ValueOrErrorAwaitable<T, Es...>{std::move(o)};
}

namespace std {

template <typename T, typename... Es, typename... Args>
struct coroutine_traits<::voe::ValueOrError<T, Es...>, Args...> {
    using promise_type = ::voe::detail::ValueOrErrorPromise<T, Es...>;
};

}  // namespace std
