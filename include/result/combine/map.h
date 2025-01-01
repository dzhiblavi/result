#pragma once

#include "result/traits.h"

namespace result {

namespace pipe {

template <typename F>
struct [[nodiscard]] Map {
    F user;

    template <typename V>
    using U = typename std::invoke_result_t<F, V>;

    explicit Map(F u) : user(std::move(u)) {}

    template <SomeResult R, typename Self>
    auto pipe(this Self&& self, R r) {
        using V = typename R::value_type;
        using Ret = typename R::template RebindValue<U<V>>;

        return std::move(r).safeVisit(detail::Overloaded{
            [&](val_tag_t, auto value) -> Ret {
                return std::forward<Self>(self).user(std::move(value));
            },
            [&](auto error) -> Ret { return makeError(std::move(error)); },
        });
    }
};

}  // namespace pipe

// Result<T, Es...> -> (T -> U) -> Result<U, Es...>
template <typename F>
auto map(F user) {
    return pipe::Map{std::move(user)};
}

}  // namespace result
