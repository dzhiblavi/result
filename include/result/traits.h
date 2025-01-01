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

template <typename R>
struct SomeResult : std::false_type {};

template <typename V, typename... Es>
struct SomeResult<Result<V, Es...>> : std::true_type {};

}  // namespace detail

template <typename R>
concept SomeResult = detail::SomeResult<R>::value;

template <typename R, typename V>
concept ResultOf = SomeResult<R> && std::is_same_v<typename R::value_type, V>;

template <typename R>
using ErrorTypesOf = detail::ErrorTypesOf<R>::type;

template <typename R>
using ValueTypeOf = detail::ValueTypeOf<R>::type;

}  // namespace result
