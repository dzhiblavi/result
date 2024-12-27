#pragma once

#include "result/detail/coro/return_object_holder.h"
#include "result/result.h"

#include <coroutine>

namespace result::detail {

template <typename T, typename... Es>
struct ResultPromiseBase {
    ResultPromiseBase() = default;

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

    detail::ReturnedObjectHolder<Result<T, Es...>>* owner = nullptr;
};

template <typename T, typename... Es>
struct ResultPromise : ResultPromiseBase<T, Es...> {
    ResultPromise() = default;

    void return_value(T x) {  // NOLINT
        assert(this->owner);
        this->owner->assign(std::move(x));
    }

    void return_value(Result<T, Es...> x) {  // NOLINT
        assert(this->owner);
        this->owner->assign(std::move(x));
    }
};

}  // namespace result::detail
