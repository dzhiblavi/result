#pragma once

#include "result/detail/helpers.h"
#include "result/detail/templates.h"
#include "result/detail/type_list.h"

#include <cassert>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>

namespace result {

template <typename ValueType, typename... ErrorTypes>
class [[nodiscard]] Result;

}  // namespace result

namespace result::detail {

namespace list = ::util::list;

namespace impl {

template <typename FromTemplate, template <typename...> class ToTemplate, typename... HeadArgs>
struct TransferTemplate;

template <
    template <typename...> class FromTemplate,
    template <typename...> class ToTemplate,
    typename... Args,
    typename... HeadArgs>
struct TransferTemplate<FromTemplate<Args...>, ToTemplate, HeadArgs...> {
    using type = ToTemplate<HeadArgs..., Args...>;
};

template <typename Callable, typename... Types>
struct VisitInvokeResult;

template <typename Callable, typename T, typename... Types>
struct VisitInvokeResult<Callable, T, Types...> {
    using type = std::invoke_result_t<Callable, T>;
};

template <typename Callable>
struct VisitInvokeResult<Callable> {
    using type = std::invoke_result_t<Callable>;
};

template <typename T>
struct ErrorTypes {
    using type = list::list<T>;
};

template <typename ValueType, typename... ETs>
struct ErrorTypes<Result<ValueType, ETs...>> {
    using type = list::list<ETs...>;
};

template <uint64_t NumVariants, bool Fits, typename... Types>
struct MinimalSizedIndexType;

template <uint64_t NumVariants, typename Type, typename... Types>
struct MinimalSizedIndexType<NumVariants, true, Type, Types...> {
    using type = Type;
};

template <uint64_t NumVariants, typename Type, typename... Types>
struct MinimalSizedIndexType<NumVariants, false, Type, Types...> {
    using first = list::get<list::list<Types...>, 0>;
    using type = typename MinimalSizedIndexType<
        NumVariants,
        std::numeric_limits<first>::max() >= NumVariants,
        Types...>::type;
};

template <typename FromVariadic, typename ToVariadic>
struct Convertible : public std::false_type {};

template <
    typename FromValueType,
    typename... FromErrorTypes,
    typename ValueType,
    typename... ErrorTypes>
struct Convertible<
    list::list<FromValueType, FromErrorTypes...>,
    list::list<ValueType, ErrorTypes...>> {
    static constexpr bool value =
        (std::is_same_v<void, FromValueType> || std::is_same_v<ValueType, void> ||
         std::is_same_v<ValueType, FromValueType>) &&
        (list::set::subsetOf<list::list<FromErrorTypes...>, list::list<ErrorTypes...>>);
};

template <
    typename Ref,
    template <class...> class OnLvalueReference,
    template <class...> class OnRvalueReference,
    typename... Args>
struct ReferenceSelector;

template <
    typename T,
    template <class...> class OnLvalueReference,
    template <class...> class OnRvalueReference,
    typename... Args>
struct ReferenceSelector<T&, OnLvalueReference, OnRvalueReference, Args...> {
    using type = OnLvalueReference<Args...>;
};

template <
    typename T,
    template <class...> class OnLvalueReference,
    template <class...> class OnRvalueReference,
    typename... Args>
struct ReferenceSelector<T&&, OnLvalueReference, OnRvalueReference, Args...> {
    using type = OnRvalueReference<Args...>;
};

template <typename VoE>
struct IsNotVoidVoePredicate;

template <typename ValueType, typename... ErrorTypes>
struct IsNotVoidVoePredicate<Result<ValueType, ErrorTypes...>>
    : public std::negation<std::is_same<ValueType, void>> {};

}  // namespace impl

template <typename FromTemplate, template <typename...> class ToTemplate, typename... HeadArgs>
using TransferTemplate =
    typename impl::TransferTemplate<FromTemplate, ToTemplate, HeadArgs...>::type;

template <typename Callable, typename... Types>
using VisitInvokeResult = typename impl::VisitInvokeResult<Callable, Types...>::type;

template <typename T>
using ErrorTypes = typename impl::ErrorTypes<T>::type;

template <typename... TypesFrom>
struct IndexMapping {
    template <typename... TypesTo>
    struct MapTo {
        static constexpr size_t size() noexcept {
            return size_;
        }

        static constexpr size_t map(size_t index) noexcept {
            assert(index < size());
            return indices_[index];
        }

        static constexpr size_t size_ = sizeof...(TypesFrom);
        static constexpr size_t indices_[sizeof...(TypesFrom)] = {
            list::indexOf<list::list<TypesTo...>, TypesFrom>...};
    };

    template <typename... TypesTo>
    struct MapTo<list::list<TypesTo...>> : public MapTo<TypesTo...> {};
};

template <typename... TypesFrom>
struct IndexMapping<list::list<TypesFrom...>> : public IndexMapping<TypesFrom...> {};

template <size_t NumVariants>
using MinimalSizedIndexType = typename impl::MinimalSizedIndexType<
    NumVariants,
    std::numeric_limits<uint8_t>::max() >= NumVariants,
    uint8_t,
    uint16_t,
    uint32_t,
    uint64_t>::type;

template <
    typename Ref,                                 //
    template <class...> class OnLvalueReference,  //
    template <class...> class OnRvalueReference,  //
    typename... Args>
using ReferenceSelector =
    typename impl::ReferenceSelector<Ref, OnLvalueReference, OnRvalueReference, Args...>::type;

template <typename... Errors>
static constexpr bool AllDecayed = (... && std::is_same_v<Errors, std::decay_t<Errors>>);

template <typename FromVariadic, typename ToVariadic>
using ConvertiblePredicate = impl::Convertible<FromVariadic, ToVariadic>;

template <typename FromVariadic, typename ToVariadic>
static constexpr bool Convertible = ConvertiblePredicate<FromVariadic, ToVariadic>::value;

template <typename ValueType>
struct IsNotVoidPredicate : public std::negation<std::is_same<ValueType, void>> {};

template <typename VoE>
using IsNotVoidVoePredicate = impl::IsNotVoidVoePredicate<VoE>;

}  // namespace result::detail
