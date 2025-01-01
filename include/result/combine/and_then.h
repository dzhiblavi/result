#pragma once

#include "result/detail/overloaded.h"
#include "result/traits.h"
#include "result/union.h"

namespace result {

namespace pipe {

template <typename F>
struct [[nodiscard]] AndThen {
    F user;

    template <typename V>
    using RU = typename std::invoke_result_t<F, V>;

    explicit AndThen(F u) : user(std::move(u)) {}

    template <SomeResult R, typename Self>
    auto pipe(this Self&& self, R r) {
        using V = typename R::value_type;
        using U = typename RU<V>::value_type;
        using Ret = Union<U, RU<V>, R>;

        return std::move(r).safeVisit(detail::Overloaded{
            [&](val_tag_t, auto value) -> Ret {
                return std::forward<Self>(self).user(std::move(value));
            },
            [&](auto error) -> Ret { return makeError(std::move(error)); },
        });
    }
};

}  // namespace pipe

// Result<T, Es...> -> (T -> Result<U, Gs...>) -> Result<U, Es..., Gs...>
template <typename F>
auto andThen(F user) {
    return pipe::AndThen{std::move(user)};
}

}  // namespace result
