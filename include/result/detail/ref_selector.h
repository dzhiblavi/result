#pragma once

namespace result::detail {

namespace impl {

template <typename Ref, typename OnLvalueReference, typename OnRvalueReference>
struct RefSelector;

template <typename T, typename OnLvalueReference, typename OnRvalueReference>
struct RefSelector<T&, OnLvalueReference, OnRvalueReference> {
    using type = OnLvalueReference;
};

template <typename T, typename OnLvalueReference, typename OnRvalueReference>
struct RefSelector<T&&, OnLvalueReference, OnRvalueReference> {
    using type = OnRvalueReference;
};

}  // namespace impl

template <typename Ref, typename OnLvalueReference, typename OnRvalueReference>
using RefSelector = impl::RefSelector<Ref, OnLvalueReference, OnRvalueReference>::type;

}  // namespace result::detail
