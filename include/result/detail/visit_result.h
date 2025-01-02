#pragma once

#include "result/detail/propagate_const.h"

#include <type_traits>

namespace result::detail {

namespace impl {

template <typename FromVoid, typename Callable, typename... Types>
struct VisitInvokeResult;

template <typename FromVoid, typename Callable, typename T, typename... Types>
struct VisitInvokeResult<FromVoid, Callable, T, Types...> {
    using type = std::invoke_result_t<Callable, propagateConst<FromVoid, T>&>;
};

template <typename FromVoid, typename Callable>
struct VisitInvokeResult<FromVoid, Callable> {
    using type = std::invoke_result_t<Callable>;
};

}  // namespace impl

template <typename FromVoid, typename Callable, typename... Types>
using VisitInvokeResult = typename impl::VisitInvokeResult<FromVoid, Callable, Types...>::type;

}  // namespace result::detail
