#pragma once

#include "result/detail/type_list/list.h"

#include <cstddef>
#include <tuple>

namespace util::list {

static constexpr size_t npos = size_t(-1);

template <typename Tl>
static constexpr size_t size = impl::Size<Tl>::value;

template <typename Tl, typename T>
static constexpr bool contains = impl::Contains<Tl, T>::value;

template <typename Tl, typename T>
static constexpr size_t indexOf = contains<Tl, T> ? impl::IndexOf<T, Tl>::value : npos;

template <typename Tl, size_t Index>
using get = std::tuple_element_t<Index, typename Tl::tuple>;

template <typename... TLs>
using concat = typename impl::Concat<TLs...>::type;

template <typename TL, typename T>
using push_back = concat<TL, list<T>>;

template <typename TL, typename T>
using push_front = concat<list<T>, TL>;

template <typename... TLs>
using flatten = typename impl::Flatten<TLs...>::type;

template <typename TL, template <typename> typename Predicate>
using filter = typename impl::Filter<TL, Predicate>::type;

template <template <typename...> typename Mapper, typename TL>
using map = typename impl::Map<Mapper, TL>::type;

template <typename TL, size_t Count>
using prefix = typename impl::Prefix<TL, Count>::type;

namespace set {

template <typename... TLs>
using product = typename impl::Product<TLs...>::type;

template <typename TL>
using unique = typename impl::Unique<TL>::type;

template <typename... TLs>
using intersect = typename impl::Intersect<TLs...>::type;

template <typename... TLs>
using unite = unique<concat<TLs...>>;

template <typename From, typename TL>
using subtract = typename impl::Subtract<From, TL>::type;

template <typename Tl>
using powerset = typename impl::Powerset<Tl>::type;

template <typename T, typename U>
static constexpr bool subsetOf = impl::SubsetOf<T, U>::value;

template <typename Tl>
static constexpr bool isaSet = size<Tl> == size<unique<Tl>>;

}  // namespace set

}  // namespace util::list
