#pragma once

#include "result/detail/coro/promise.h"

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
        return std::move(object).getValue();
    }

    template <typename U>
    void await_suspend(std::coroutine_handle<ResultPromise<U, Es...>> h) {  // NOLINT
        auto&& promise = h.promise();
        auto* owner = promise.owner;
        assert(owner);
        owner->assign(std::move(object));
    }
};

}  // namespace result::detail
