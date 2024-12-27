#pragma once

#include "voe/detail/type_list/front.h"

#include <type_traits>
#include <utility>

namespace util::list::impl {

template <typename TL>
struct Size;

template <typename... Ts>
struct Size<list<Ts...>> : public std::integral_constant<size_t, sizeof...(Ts)> {};

template <typename TL, typename T>
struct Contains;

template <typename T, typename... Ts>
struct Contains<list<Ts...>, T>
    : public std::integral_constant<bool, (std::is_same_v<T, Ts> || ...)> {};

template <typename... TLs>
struct Concat;

template <>
struct Concat<> {
    using type = list<>;
};

template <typename... T1, typename... T2>
struct Concat<list<T1...>, list<T2...>> {
    using type = list<T1..., T2...>;
};

template <typename TL, typename... TLs>
struct Concat<TL, TLs...> {
    using type = typename Concat<TL, typename Concat<TLs...>::type>::type;
};

template <typename TL>
struct Flatten;

template <typename TL, typename... TLs>
struct Flatten<list<TL, TLs...>> {
    using rest = typename Flatten<list<TLs...>>::type;
    using type = typename Concat<TL, rest>::type;
};

template <>
struct Flatten<list<>> {
    using type = list<>;
};

template <typename T>
struct SameTypePredicate {
    template <typename U>
    using type = std::is_same<U, T>;
};

template <typename TL, template <typename> typename Predicate>
struct Filter;

template <template <typename> typename Predicate>
struct Filter<list<>, Predicate> {
    using type = list<>;
};

template <template <typename> typename Predicate, typename T, typename... Ts>
struct Filter<list<T, Ts...>, Predicate> {
    using rest = typename Filter<list<Ts...>, Predicate>::type;
    using type =
        std::conditional_t<Predicate<T>::value, typename Concat<list<T>, rest>::type, rest>;
};

template <typename TL1, typename TL2>
struct Product2;

template <typename TL2>
struct Product2<list<>, TL2> {
    using type = list<>;
};

template <typename TL1, typename... TLs>
struct Product2<list<TL1, TLs...>, list<>> {
    using type = list<>;
};

template <typename E, typename... Es, typename... TLs>
struct Product2<list<E, Es...>, list<TLs...>> {
    using right = list<TLs...>;
    using rest = typename Product2<list<Es...>, right>::type;
    using type = typename Concat<list<typename Concat<list<E>, TLs>::type>..., rest>::type;
};

template <typename... TLs>
struct Product;

template <typename... Ts>
struct Product<list<Ts...>> {
    using type = list<list<Ts>...>;
};

template <typename TL, typename... TLs>
struct Product<TL, TLs...> {
    using rest = typename Product<TLs...>::type;
    using type = typename Product2<TL, rest>::type;
};

template <typename TL>
struct Unique;

template <>
struct Unique<list<>> {
    using type = list<>;
};

template <typename T, typename... Ts>
struct Unique<list<T, Ts...>> {
    using rest = typename Unique<list<Ts...>>::type;
    using type = std::
        conditional_t<Contains<list<Ts...>, T>::value, rest, typename Concat<list<T>, rest>::type>;
};

template <typename TL1, typename TL2>
struct Intersect2;

template <typename TL1>
struct Intersect2<TL1, list<>> {
    using type = list<>;
};

template <typename TL1, typename T, typename... Ts>
struct Intersect2<TL1, list<T, Ts...>> {
    using rest = typename Intersect2<TL1, list<Ts...>>::type;
    using type =
        std::conditional_t<Contains<TL1, T>::value, typename Concat<rest, list<T>>::type, rest>;
};

template <typename... TLs>
struct Intersect;

template <typename TL>
struct Intersect<TL> {
    using type = TL;
};

template <typename TL, typename... TLs>
struct Intersect<TL, TLs...> {
    using rest = typename Intersect<TLs...>::type;
    using type = typename Intersect2<TL, rest>::type;
};

template <typename T1, typename T2>
struct Subtract;

template <typename TL>
struct Subtract<list<>, TL> {
    using type = list<>;
};

template <typename TL, typename T, typename... Ts>
struct Subtract<list<T, Ts...>, TL> {
    using rest = typename Subtract<list<Ts...>, TL>::type;
    using type =
        std::conditional_t<Contains<TL, T>::value, rest, typename Concat<rest, list<T>>::type>;
};

template <template <typename...> class Mapper, typename TL>
struct Map;

template <template <typename...> class Mapper>
struct Map<Mapper, list<>> {
    using type = list<>;
};

template <template <typename...> class Mapper, typename T, typename... Tx>
struct Map<Mapper, list<T, Tx...>> {
    using rest = typename Map<Mapper, list<Tx...>>::type;
    using type = typename Concat<list<Mapper<T>>, rest>::type;
};

template <typename TL>
struct Powerset;

template <>
struct Powerset<list<>> {
    using type = list<list<>>;
};

template <typename T, typename... Tx>
struct Powerset<list<T, Tx...>> {
    template <typename List>
    using Mapper = typename Concat<list<T>, List>::type;
    using PowersetRest = typename Powerset<list<Tx...>>::type;

    using type = typename Concat<PowersetRest, typename Map<Mapper, PowersetRest>::type>::type;
};

template <typename T, typename... Ts>
struct IndexOf<T, list<Ts...>> : std::integral_constant<size_t, size_t(-1)> {};

template <typename T, typename... Ts>
struct IndexOf<T, list<T, Ts...>> : std::integral_constant<size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct IndexOf<T, list<U, Ts...>>
    : std::integral_constant<size_t, 1 + IndexOf<T, list<Ts...>>::value> {};

template <typename... Ts1, typename... Ts2>
struct SubsetOf<list<Ts1...>, list<Ts2...>>
    : public std::bool_constant<(... && contains<list<Ts2...>, Ts1>)> {};

template <size_t Count>
struct Prefix<list<>, Count> : std::enable_if<(Count == 0), list<>> {};

template <typename... Ts>
struct Prefix<list<Ts...>, 0> {
    using type = list<>;
};

template <typename T, typename... Ts, size_t Count>
requires(Count > 0)
struct Prefix<list<T, Ts...>, Count> {
    using type = push_front<prefix<list<Ts...>, Count - 1>, T>;
};

}  // namespace util::list::impl
