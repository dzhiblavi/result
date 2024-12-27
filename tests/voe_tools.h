#pragma once

#include "voe/detail/templates.h"
#include "voe/detail/type_list.h"
#include "voe/value_or_error.h"

namespace test {

namespace list = ::util::list;
namespace tpl = ::util::tpl;

template <typename ValueType, typename... ErrorTypes>
using Types = list::map<
    tpl::transferTo<tpl::bindFront<::voe::ValueOrError, ValueType>::template TN>::template Transfer,
    list::set::powerset<list::list<ErrorTypes...>>>;

template <typename ValueType, typename ValueOrError>
struct HasAnyErrorPredicate
    : public std::negation<std::is_same<::voe::ValueOrError<ValueType>, ValueOrError>> {};

template <typename ValueType, typename... ErrorTypes>
using TypesWithErrors = list::filter<
    Types<ValueType, ErrorTypes...>,
    tpl::bindFront<HasAnyErrorPredicate, ValueType>::template T1>;

template <typename ValueOrErrorPair>
struct ConvertiblePredicate;

template <typename ValueOrError1, typename ValueOrError2>
struct ConvertiblePredicate<list::list<ValueOrError1, ValueOrError2>>
    : public voe::detail::ConvertiblePredicate<
          voe::detail::TransferTemplate<ValueOrError1, voe::detail::list::list>,
          voe::detail::TransferTemplate<ValueOrError2, voe::detail::list::list>> {};

template <typename ValueType, typename... ErrorTypes>
requires(!std::is_same_v<void, ValueType>)
using AllConvertiblePairs = list::filter<
    list::concat<
        list::set::product<Types<ValueType, ErrorTypes...>, Types<ValueType, ErrorTypes...>>,
        list::set::product<Types<void, ErrorTypes...>, Types<ValueType, ErrorTypes...>>,
        list::set::product<Types<ValueType, ErrorTypes...>, Types<void, ErrorTypes...>>>,
    ConvertiblePredicate>;

template <typename Functor, typename... Types>
void InstantiateAndCall(list::list<Types...>) {
    (Functor::call(static_cast<Types*>(nullptr)), ...);
};

namespace unit {

static_assert(std::is_same_v<Types<void>, list::list<::voe::VoidOrError<>>>);
static_assert(
    std::is_same_v<Types<void, int>, list::list<voe::VoidOrError<>, voe::VoidOrError<int>>>);
static_assert(std::is_same_v<
              Types<void, int, float>,
              list::list<
                  voe::VoidOrError<>,
                  voe::VoidOrError<float>,
                  voe::VoidOrError<int>,
                  voe::VoidOrError<int, float>>>);
static_assert(std::is_same_v<Types<int>, list::list<voe::ValueOrError<int>>>);
static_assert(std::is_same_v<
              Types<int, float>,
              list::list<voe::ValueOrError<int>, voe::ValueOrError<int, float>>>);
static_assert(std::is_same_v<
              Types<void, int, float>,
              list::list<
                  voe::VoidOrError<>,
                  voe::VoidOrError<float>,
                  voe::VoidOrError<int>,
                  voe::VoidOrError<int, float>>>);

static_assert(std::is_same_v<TypesWithErrors<void>, list::list<>>);
static_assert(std::is_same_v<TypesWithErrors<void, int>, list::list<voe::VoidOrError<int>>>);
static_assert(
    std::is_same_v<
        TypesWithErrors<void, int, float>,
        list::list<voe::VoidOrError<float>, voe::VoidOrError<int>, voe::VoidOrError<int, float>>>);
static_assert(std::is_same_v<TypesWithErrors<int>, list::list<>>);
static_assert(
    std::is_same_v<TypesWithErrors<int, float>, list::list<voe::ValueOrError<int, float>>>);
static_assert(
    std::is_same_v<
        TypesWithErrors<void, int, float>,
        list::list<voe::VoidOrError<float>, voe::VoidOrError<int>, voe::VoidOrError<int, float>>>);

static_assert(std::is_same_v<
              AllConvertiblePairs<int, char>,
              list::list<
                  list::list<voe::ValueOrError<int>, voe::ValueOrError<int>>,
                  list::list<voe::ValueOrError<int>, voe::ValueOrError<int, char>>,
                  list::list<voe::ValueOrError<int, char>, voe::ValueOrError<int, char>>,
                  list::list<voe::ValueOrError<void>, voe::ValueOrError<int>>,
                  list::list<voe::ValueOrError<void>, voe::ValueOrError<int, char>>,
                  list::list<voe::ValueOrError<void, char>, voe::ValueOrError<int, char>>,
                  list::list<voe::ValueOrError<int>, voe::ValueOrError<void>>,
                  list::list<voe::ValueOrError<int>, voe::ValueOrError<void, char>>,
                  list::list<voe::ValueOrError<int, char>, voe::ValueOrError<void, char>>>>);

}  // namespace unit

}  // namespace test
