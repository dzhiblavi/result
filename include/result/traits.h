#pragma once

#include "result/result.h"

namespace result {

namespace detail {

template <typename R>
struct ErrorTypesOf;

template <typename V, typename... Es>
struct ErrorTypesOf<Result<V, Es...>> {
    using type = tl::List<Es...>;
};

template <typename R>
struct ValueTypeOf;

template <typename V, typename... Es>
struct ValueTypeOf<Result<V, Es...>> {
    using type = V;
};

}  // namespace detail

template <typename R>
using ErrorTypesOf = detail::ErrorTypesOf<R>::type;

template <typename R>
using ValueTypeOf = detail::ValueTypeOf<R>::type;

}  // namespace result
