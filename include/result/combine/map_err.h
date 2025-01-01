#pragma once

#include "result/combine/combinator.h"
#include "result/detail/apply_to_template.h"
#include "result/result.h"
#include "result/traits.h"

namespace result {

namespace pipe {

template <typename F>
struct [[nodiscard]] MapErr {
    // F: Es... -> Gs... (multiple overloads)
    F user;

    template <typename R>
    using Es = ErrorTypesOf<R>;  // tl::List

    struct ErrMapper {
        template <typename E>
        using Map = std::invoke_result_t<F, E>;
    };

    template <typename R>
    using Gs = tl::Map<ErrMapper, Es<R>>;

    explicit MapErr(F u) : user(std::move(u)) {}

    template <SomeResult R, typename Self>
    auto pipe(this Self&& self, R r) {
        using V = typename R::value_type;
        using Ret = detail::ApplyToTemplate<Result, tl::PushFront<Gs<R>, V>>;

        return std::move(r).safeVisit(detail::Overloaded{
            [](val_tag_t, V value) -> Ret { return std::move(value); },
            [&](auto err) -> Ret {
                return makeError(std::forward<Self>(self).user(std::move(err)));
            },
        });
    }
};

}  // namespace pipe

// Result<T, Es...> -> (Es... -> Gs...) -> Result<T, Gs...>
template <typename F>
Combinator auto mapErr(F user) {
    return pipe::MapErr{std::move(user)};
}

}  // namespace result
