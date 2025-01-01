#pragma once

#include <type_traits>

namespace result::detail {

namespace impl {

template <typename Callable, typename... Types>
struct VisitInvokeResult;

template <typename Callable, typename T, typename... Types>
struct VisitInvokeResult<Callable, T, Types...> {
    using type = std::invoke_result_t<Callable, T>;
};

template <typename Callable>
struct VisitInvokeResult<Callable> {
    using type = std::invoke_result_t<Callable>;
};

}  // namespace impl

template <typename Callable, typename... Types>
using VisitInvokeResult = typename impl::VisitInvokeResult<Callable, Types&...>::type;

}  // namespace result::detail
