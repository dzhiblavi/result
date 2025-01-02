#pragma once

#include "result/detail/propagate_const.h"
#include "result/detail/visit_result.h"

#include <type_traits>
#include <utility>

namespace result::detail {

template <typename FromVoid, typename Callable, typename... Types>
struct CallableFunctorArray {
    using ResultType = detail::VisitInvokeResult<FromVoid, Callable, Types...>;

    template <typename F>
    static constexpr ResultType call(F&& func, FromVoid* ptr, size_t index) {
        return Array[index](std::forward<F>(func), ptr);
    }

 private:
    template <typename Type>
    struct Call {
        using PointerType = propagateConst<Type, void>;

        static constexpr decltype(auto) call(Callable callable, PointerType* ptr) {
            return std::forward<Callable>(callable)(*static_cast<Type*>(ptr));
        }
    };

    using Fn = ResultType (*)(Callable, FromVoid*);

    static constexpr Fn Array[sizeof...(Types)] = {
        Call<propagateConst<FromVoid, Types>>::call...,
    };
};

template <typename T>
concept SelfPtr = std::is_same_v<std::decay_t<T>, void>;

template <typename... Ts>
struct VTable {
    template <typename F, SelfPtr S>
    static constexpr decltype(auto) dispatch(F&& f, S* self, size_t index) {
        return CallableFunctorArray<S, F, Ts...>::call(std::forward<F>(f), self, index);
    }
};

}  // namespace result::detail
