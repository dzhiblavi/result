#pragma once

#include "result/detail/constexpr_for.h"
#include "result/value_or_error.h"

#include <tuple>

namespace result {

/**
 * @brief Combine two Result types
 *
 * For example, consider the following snippet:
 * @code
 * using A = Result<char, int, short>;
 * A Foo() { ... }
 *
 * using B = Result<void, long, int>;
 * B Bar() { ... }
 *
 * using C = Union<float, A, B>;
 * static_assert(std::is_same_v<C, Result<float, int, short, long>>);
 * @endcode
 */
template <typename ValueType, typename... VoEOrErrorTypes>
using Union = detail::TransferTemplate<
    detail::list::set::unite<detail::ErrorTypes<VoEOrErrorTypes>...>,
    Result,
    ValueType>;

namespace detail {

template <typename VoE, typename... VoEs>
Union<void, std::decay_t<VoE>, std::decay_t<VoEs>...> firstError(VoE& voe, VoEs&... voes) {
    if (voe.HasAnyError()) {
        return std::move(voe).DiscardValue();
    }

    if constexpr (sizeof...(voes) == 0) {
        return {};
    } else {
        return firstError(voes...);
    }
}

}  // namespace detail

template <typename... VoEs>
using InvertResultType = Union<
    util::tpl::applyToTemplate<
        std::tuple,
        util::list::
            filter<util::list::list<typename VoEs::value_type...>, detail::IsNotVoidPredicate>>,
    VoEs...>;

/**
 * @brief Convert multiple VoEs into one, i.e. something like
 *        (VoE<A, E1, E2>, VoE<B, E2, E3>) -> VoE<tuple<A, B>, E1, E2, E3>
 * @note If any of provided VoEs hold an error, the resulting one will hold the first error (in
 * tuple order) And all values will be dropped.
 */
template <typename... VoEs>
InvertResultType<std::decay_t<VoEs>...> invert(std::tuple<VoEs...>&& voes_tpl) noexcept {
    using namespace util::list;
    using namespace util::tpl;

    if (auto error = std::apply(detail::firstError<VoEs...>, voes_tpl); error.HasAnyError())
        [[unlikely]] {
        return error;
    }

    using Voes = list<std::decay_t<VoEs>...>;
    using ValueVoes = filter<Voes, detail::IsNotVoidVoePredicate>;
    using ValueVoesTuple = applyToTemplate<std::tuple, ValueVoes>;

    // should have default "empty" state constructor
    ValueVoesTuple voe_tuple{};

    detail::constexprFor<size_t{0}, size<Voes>, size_t{1}>(
        [&, source = std::move(voes_tpl)](auto sourceIndex) mutable {
            if constexpr (!std::is_same_v<typename get<Voes, sourceIndex>::value_type, void>) {
                constexpr size_t destIndex =
                    size<filter<prefix<Voes, sourceIndex.value>, detail::IsNotVoidVoePredicate>>;
                std::get<destIndex>(voe_tuple) = std::move(std::get<sourceIndex>(source));
            }
        });

    // convert std::tuple<Voe<x>, ...> -> std::tuple<x...>;
    return std::apply(
        [](auto&&... voes) { return std::make_tuple(std::move(voes.GetValue())...); },
        std::move(voe_tuple));
}

template <typename... VoEs>
auto invert(VoEs&&... voes) noexcept {
    return invert(std::forward_as_tuple(voes...));
}

}  // namespace result
