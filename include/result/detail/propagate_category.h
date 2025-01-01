#pragma once

namespace result::detail {

namespace impl {

template <typename T, typename U>
struct PropagateCategory;

template <typename T, typename U>
struct PropagateCategory<T&, U> {
    using type = U&;
};

template <typename T, typename U>
struct PropagateCategory<T&&, U> {
    using type = U&&;
};

template <typename T, typename U>
struct PropagateCategory<const T&, U> {
    using type = const U&;
};

template <typename T, typename U>
struct PropagateCategory<const T&&, U> {
    using type = const U&&;
};

}  // namespace impl

template <typename T, typename U>
using propagateCategory = typename impl::PropagateCategory<T, U>::type;

}  // namespace result::detail
