#pragma once

#include "result/detail/apply_to_template.h"
#include "result/result.h"
#include "result/traits.h"

#include <type_list/list.h>

namespace test {

using namespace result;

template <typename... Es>
using AllErrorSubsets = tl::Powerset<tl::List<Es...>>;

template <typename V>
struct ResultErrorsMapper {
    template <typename ErrorsList>
    using Map = result::detail::ApplyToTemplate<Result, tl::PushFront<ErrorsList, V>>;
};

template <typename V, typename... Es>
using Types = tl::Map<ResultErrorsMapper<V>, AllErrorSubsets<Es...>>;

struct HasAnyErrorPredicate {
    template <typename R>
    static constexpr bool test() noexcept {
        return !tl::Empty<ErrorTypesOf<R>>;
    }
};

template <typename V, typename... Es>
using TypesWithErrors = tl::Filter<HasAnyErrorPredicate, Types<V, Es...>>;

struct ConvertiblePredicate {
    template <typename Rs>
    static constexpr bool test() noexcept {
        return tl::apply(
            []<typename R1, typename R2>(tl::Type<R1>, tl::Type<R2>) {
                return std::is_convertible_v<R1, R2>;
            },
            Rs{});
    }
};

template <typename V, typename... Es>
using AllConvertiblePairs =
    tl::Filter<ConvertiblePredicate, tl::Prod<Types<V, Es...>, Types<V, Es...>>>;

template <typename Functor, typename... Types>
void instantiateAndCall(tl::List<Types...>) {
    (Functor::call(static_cast<Types*>(nullptr)), ...);
};

namespace unit {

static_assert(std::is_same_v<Types<int>, tl::List<Result<int>>>);
static_assert(std::is_same_v<Types<float, int>, tl::List<Result<float, int>, Result<float>>>);

static_assert(
    std::is_same_v<
        Types<int, int, float>,
        tl::List<Result<int, int, float>, Result<int, int>, Result<int, float>, Result<int>>>);

static_assert(std::is_same_v<TypesWithErrors<int>, tl::List<>>);
static_assert(std::is_same_v<TypesWithErrors<int, int>, tl::List<Result<int, int>>>);

static_assert(std::is_same_v<
              AllConvertiblePairs<int, char>,
              tl::List<
                  tl::List<Result<int, char>, Result<int, char>>,
                  tl::List<Result<int>, Result<int, char>>,
                  tl::List<Result<int>, Result<int>>>>);

}  // namespace unit

}  // namespace test
