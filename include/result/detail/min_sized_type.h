#pragma once

#include <type_list/list.h>

namespace result::detail {

namespace impl {

template <uint64_t NumVariants, std::integral... Types>
struct MinimalSizedIndexType {
    using type = void;
};

template <uint64_t NumVariants, std::integral Type, std::integral... Types>
struct MinimalSizedIndexType<NumVariants, Type, Types...> {
    using type = std::conditional_t<
        std::numeric_limits<Type>::max() >= NumVariants,
        Type,
        typename MinimalSizedIndexType<NumVariants, Types...>::type>;
};

}  // namespace impl

template <size_t NumVariants>
using MinimalSizedIndexType =
    typename impl::MinimalSizedIndexType<NumVariants, uint8_t, uint16_t, uint32_t, uint64_t>::type;

}  // namespace result::detail
