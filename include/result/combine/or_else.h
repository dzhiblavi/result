#pragma once

#include "result/detail/apply_to_template.h"
#include "result/detail/overloaded.h"
#include "result/traits.h"

namespace result {

namespace pipe {

template <typename F>
struct [[nodiscard]] OrElse {
    // Es... -> Result<T, Gs...>
    F user;

    template <typename R>
    using Es = ErrorTypesOf<R>;

    struct ErrMapper {
        template <typename E>
        using Map = ErrorTypesOf<std::invoke_result_t<F, E>>;
    };

    template <typename R>
    using GsThick = tl::Map<ErrMapper, Es<R>>;  // List<List<Gs...>...>

    template <typename R>
    using Gs = tl::Unique<tl::Flatten<GsThick<R>>>;

    explicit OrElse(F u) : user(std::move(u)) {}

    template <SomeResult R, typename Self>
    auto pipe(this Self&& self, R r) {
        using V = typename R::value_type;
        using Ret = detail::ApplyToTemplate<Result, tl::PushFront<Gs<R>, V>>;

        return std::move(r).taggedVisit(detail::Overloaded{
            [](val_tag_t, V value) -> Ret { return std::move(value); },
            [&](auto err) -> Ret { return std::forward<Self>(self).user(std::move(err)); },
        });
    }
};

}  // namespace pipe

// Result<T, Es...> -> (Es... -> Result<T, Gs...>) -> Result<T, Gs...>
template <typename F>
auto orElse(F user) {
    return pipe::OrElse{std::move(user)};
}

}  // namespace result
