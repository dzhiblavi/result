#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace voe::detail {

template <auto Start, auto End, auto Inc, typename F>
constexpr void constexprFor(F&& f) {
    if constexpr (Start < End) {
        f(std::integral_constant<decltype(Start), Start>());
        constexprFor<Start + Inc, End, Inc>(std::forward<F>(f));
    }
}

template <typename F, typename... Args>
constexpr void constexprFor(const F& f, Args&&... args) {
    (f(std::forward<Args>(args)), ...);
}

template <typename F, typename TupleLike>
constexpr void constexprFor(F&& f, TupleLike&& tuple) {
    std::apply(
        [f = std::forward<F>(f)]<typename... Types>(Types&&... values) {
            constexprFor(std::forward<F>(f), std::forward<Types>(values)...);
        },
        std::forward<TupleLike>(tuple));
}

}  // namespace voe::detail
