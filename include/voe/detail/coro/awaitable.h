#pragma once

#include "voe/detail/coro/promise.h"

#include <coroutine>

namespace voe::detail {

template <typename T, typename... Es>
struct ValueOrErrorAwaitable {
    ValueOrError<T, Es...> object;

    bool await_ready() noexcept {  // NOLINT
        return !object.HasAnyError();
    }

    T await_resume() {  // NOLINT
        assert(!object.HasAnyError());
        return std::move(object).GetValue();
    }

    template <typename U>
    void await_suspend(std::coroutine_handle<ValueOrErrorPromise<U, Es...>> h) {  // NOLINT
        auto&& promise = h.promise();
        auto* owner = promise.owner;
        assert(owner);
        owner->assign(std::move(object));
    }
};

}  // namespace voe::detail
