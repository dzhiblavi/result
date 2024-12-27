#pragma once

#include "voe/detail/coro/return_object_holder.h"
#include "voe/value_or_error.h"

#include <coroutine>

namespace voe::detail {

template <typename T, typename... Es>
struct ValueOrErrorPromiseBase {
    ValueOrErrorPromiseBase() = default;

    auto get_return_object() noexcept {  // NOLINT
        return detail::ReturnedObjectHolder{&owner};
    }

    auto initial_suspend() noexcept {  // NOLINT
        return std::suspend_never{};
    }

    auto final_suspend() noexcept {  // NOLINT
        return std::suspend_never{};
    }

    [[noreturn]] void unhandled_exception() noexcept {  // NOLINT
        // exceptions are not yet supported
        std::terminate();
    }

    detail::ReturnedObjectHolder<ValueOrError<T, Es...>>* owner = nullptr;
};

template <typename T, typename... Es>
struct ValueOrErrorPromise : ValueOrErrorPromiseBase<T, Es...> {
    ValueOrErrorPromise() = default;

    void return_value(T x) {  // NOLINT
        assert(this->owner);
        this->owner->assign(std::move(x));
    }

    void return_value(ValueOrError<T, Es...> x) {  // NOLINT
        assert(this->owner);
        this->owner->assign(std::move(x));
    }
};

}  // namespace voe::detail
