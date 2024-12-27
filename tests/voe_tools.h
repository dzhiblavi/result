#pragma once

#include "result/detail/templates.h"
#include "result/detail/type_list.h"
#include "result/value_or_error.h"

namespace test {

namespace list = ::util::list;
namespace tpl = ::util::tpl;

template <typename ValueType, typename... ErrorTypes>
using Types = list::map<
    tpl::transferTo<tpl::bindFront<::result::Result, ValueType>::template TN>::template Transfer,
    list::set::powerset<list::list<ErrorTypes...>>>;

template <typename ValueType, typename Result>
struct HasAnyErrorPredicate
    : public std::negation<std::is_same<::result::Result<ValueType>, Result>> {};

template <typename ValueType, typename... ErrorTypes>
using TypesWithErrors = list::filter<
    Types<ValueType, ErrorTypes...>,
    tpl::bindFront<HasAnyErrorPredicate, ValueType>::template T1>;

template <typename ResultPair>
struct ConvertiblePredicate;

template <typename Result1, typename Result2>
struct ConvertiblePredicate<list::list<Result1, Result2>>
    : public result::detail::ConvertiblePredicate<
          result::detail::TransferTemplate<Result1, result::detail::list::list>,
          result::detail::TransferTemplate<Result2, result::detail::list::list>> {};

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

static_assert(std::is_same_v<Types<void>, list::list<::result::VoidOrError<>>>);
static_assert(
    std::is_same_v<Types<void, int>, list::list<result::VoidOrError<>, result::VoidOrError<int>>>);
static_assert(std::is_same_v<
              Types<void, int, float>,
              list::list<
                  result::VoidOrError<>,
                  result::VoidOrError<float>,
                  result::VoidOrError<int>,
                  result::VoidOrError<int, float>>>);
static_assert(std::is_same_v<Types<int>, list::list<result::Result<int>>>);
static_assert(
    std::is_same_v<Types<int, float>, list::list<result::Result<int>, result::Result<int, float>>>);
static_assert(std::is_same_v<
              Types<void, int, float>,
              list::list<
                  result::VoidOrError<>,
                  result::VoidOrError<float>,
                  result::VoidOrError<int>,
                  result::VoidOrError<int, float>>>);

static_assert(std::is_same_v<TypesWithErrors<void>, list::list<>>);
static_assert(std::is_same_v<TypesWithErrors<void, int>, list::list<result::VoidOrError<int>>>);
static_assert(std::is_same_v<
              TypesWithErrors<void, int, float>,
              list::list<
                  result::VoidOrError<float>,
                  result::VoidOrError<int>,
                  result::VoidOrError<int, float>>>);
static_assert(std::is_same_v<TypesWithErrors<int>, list::list<>>);
static_assert(std::is_same_v<TypesWithErrors<int, float>, list::list<result::Result<int, float>>>);
static_assert(std::is_same_v<
              TypesWithErrors<void, int, float>,
              list::list<
                  result::VoidOrError<float>,
                  result::VoidOrError<int>,
                  result::VoidOrError<int, float>>>);

static_assert(std::is_same_v<
              AllConvertiblePairs<int, char>,
              list::list<
                  list::list<result::Result<int>, result::Result<int>>,
                  list::list<result::Result<int>, result::Result<int, char>>,
                  list::list<result::Result<int, char>, result::Result<int, char>>,
                  list::list<result::Result<void>, result::Result<int>>,
                  list::list<result::Result<void>, result::Result<int, char>>,
                  list::list<result::Result<void, char>, result::Result<int, char>>,
                  list::list<result::Result<int>, result::Result<void>>,
                  list::list<result::Result<int>, result::Result<void, char>>,
                  list::list<result::Result<int, char>, result::Result<void, char>>>>);

}  // namespace unit

}  // namespace test
