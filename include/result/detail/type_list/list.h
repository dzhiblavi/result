#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

namespace util::list {

template <typename... Args>
struct list {
    using tuple = std::tuple<Args...>;
};

using empty = list<>;

}  // namespace util::list

namespace util::list::impl {

template <typename TL>
struct Size;

template <typename TL, typename T>
struct Contains;

template <typename T, typename Tl>
struct IndexOf;

template <typename... TLs>
struct Concat;

template <typename TL>
struct Flatten;

template <typename T>
struct SameTypePredicate;

template <typename TL, template <typename> typename Predicate>
struct Filter;

template <typename TL1, typename TL2>
struct Product2;

template <typename... TLs>
struct Product;

template <typename TL>
struct Unique;

template <typename TL1, typename TL2>
struct Intersect2;

template <typename... TLs>
struct Intersect;

template <typename T1, typename T2>
struct Subtract;

template <template <typename...> class Mapper, typename TL>
struct Map;

template <typename TL>
struct Powerset;

template <typename T, typename U>
struct SubsetOf;

template <typename TL, size_t Count>
struct Prefix;

}  // namespace util::list::impl