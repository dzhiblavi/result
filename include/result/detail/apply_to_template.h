#pragma once

#include <type_list/list.h>

namespace result::detail {

namespace impl {

template <template <typename...> typename T, typename... Ts>
auto applyToTemplate(tl::List<Ts...>) -> T<Ts...>;

}  // namespace impl

template <template <typename...> typename T, typename List>
using ApplyToTemplate = decltype(impl::applyToTemplate<T>(List{}));

}  // namespace result::detail
