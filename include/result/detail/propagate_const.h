#pragma once

#include <type_traits>

namespace result::detail {

namespace impl {

template <typename From, typename To>
struct PropagateConst;

};  // namespace impl

template <typename From, typename To>
using propagateConst = typename impl::PropagateConst<From, To>::type;

}  // namespace result::detail

namespace result::detail::impl {

template <typename From, typename To>
struct PropagateConst : public std::remove_const<To> {};

template <typename From, typename To>
struct PropagateConst<const From, To> {
    using type = const To;
};

template <typename From, typename To>
struct PropagateConst<const From&, To> {
    using type = const To;
};

template <typename From, typename To>
struct PropagateConst<const From&&, To> {
    using type = const To;
};

}  // namespace result::detail::impl
