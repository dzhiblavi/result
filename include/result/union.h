#pragma once

#include "result/detail/apply_to_template.h"
#include "result/result.h"

namespace result {

namespace detail {

template <typename RorE>
struct ErrorTypesFor {
    using type = tl::List<RorE>;
};

template <typename V, typename... Es>
struct ErrorTypesFor<Result<V, Es...>> {
    using type = tl::List<Es...>;
};

}  // namespace detail

/**
 * @brief Combine two Result types
 *
 * For example, consider the following snippet:
 * @code
 * using A = Result<char, int, short>;
 * A Foo() { ... }
 *
 * using B = Result<void, long, int>;
 * B Bar() { ... }
 *
 * using C = Union<float, A, B>;
 * static_assert(std::is_same_v<C, Result<float, int, short, long>>);
 * @endcode
 */
template <typename ValueType, typename... VoEOrErrorTypes>
using Union = detail::ApplyToTemplate<
    Result,
    tl::PushFront<tl::Unite<typename detail::ErrorTypesFor<VoEOrErrorTypes>::type...>, ValueType>>;

}  // namespace result
