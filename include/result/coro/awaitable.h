#pragma once

#include "result/coro/promise.h"

#include <coroutine>

namespace result::detail {

template <typename T, typename... Es>
struct ResultAwaitable {
    Result<T, Es...> object;

    bool await_ready() noexcept {  // NOLINT
        return !object.hasAnyError();
    }

    T await_resume() {  // NOLINT
        assert(!object.hasAnyError());
        return std::move(object).value();
    }

    template <typename U, typename... Gs>
    void await_suspend(std::coroutine_handle<ResultPromise<U, Gs...>> h) {  // NOLINT
        auto&& promise = h.promise();
        auto* owner = promise.owner;
        assert(owner);

        std::move(object).safeVisit(detail::Overloaded{
            [](val_tag_t, auto) { std::unreachable(); },
            [&](auto error) { owner->assign(makeError(std::move(error))); },
        });
    }
};

}  // namespace result::detail
