#pragma once

#include "result/detail/propagate_const.h"
#include "result/detail/ref_selector.h"
#include "result/detail/visit_result.h"

#include <type_traits>
#include <utility>

namespace result::detail {

template <typename... Types>
struct DestructorFunctorArray {
    static constexpr void call(void* ptr, size_t index) noexcept {
        Array[index](ptr);
    }

 private:
    template <typename Type>
    struct Call {
        static constexpr void call(void* ptr) noexcept {
            if constexpr (!std::is_trivially_destructible_v<Type>) {
                static_cast<Type*>(ptr)->~Type();
            }
        }
    };

    using Fn = void (*)(void*);

    static constexpr Fn Array[sizeof...(Types)] = {
        Call<Types>::call...,
    };
};

template <typename FromVoid, typename Callable, typename... Types>
struct CallableFunctorArray {
    using ResultType = detail::VisitInvokeResult<Callable, Types...>;

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

template <typename FromVoid, typename... Types>
struct CopyConstructorFunctorArray {
    static constexpr void call(FromVoid* from, void* to, size_t index) {
        Array[index](from, to);
    }

 private:
    template <typename Type>
    struct Call {
        using FromType = propagateConst<Type, void>;

        static constexpr void call(FromType* from, void* to) noexcept(
            std::is_nothrow_copy_constructible_v<Type>) {
            new (to) std::remove_cv_t<Type>(*static_cast<Type*>(from));
        }
    };

    using Fn = void (*)(FromVoid*, void*);

    static constexpr Fn Array[sizeof...(Types)] = {
        Call<propagateConst<FromVoid, Types>>::call...,
    };
};

template <typename FromVoid, typename... Types>
struct CopyAssignmentFunctorArray {
    static constexpr void call(FromVoid* from, void* to, size_t index) {
        Array[index](from, to);
    }

 private:
    template <typename Type>
    struct Call {
        using FromType = propagateConst<Type, void>;

        static constexpr void call(FromType* from, void* to) noexcept(
            std::is_nothrow_copy_assignable_v<Type>) {
            *static_cast<std::remove_cv_t<Type>*>(to) = *static_cast<Type*>(from);
        }
    };

    using Fn = void (*)(FromVoid*, void*);

    static constexpr Fn Array[sizeof...(Types)] = {
        Call<propagateConst<FromVoid, Types>>::call...,
    };
};

template <typename FromVoid, typename... Types>
struct MoveConstructorFunctorArray {
    static constexpr void call(FromVoid* from, void* to, size_t index) {
        Array[index](from, to);
    }

 private:
    template <typename Type>
    struct Call {
        using FromType = propagateConst<Type, void>;

        static constexpr void call(FromType* from, void* to) noexcept(
            std::is_nothrow_move_constructible_v<Type>) {
            new (to) std::remove_cv_t<Type>(std::move(*static_cast<Type*>(from)));
        }
    };

    using Fn = void (*)(FromVoid*, void*);

    static constexpr Fn Array[sizeof...(Types)] = {
        Call<propagateConst<FromVoid, Types>>::call...,
    };
};

template <typename FromVoid, typename... Types>
struct MoveAssignmentFunctorArray {
    static constexpr void call(FromVoid* from, void* to, size_t index) {
        Array[index](from, to);
    }

 private:
    template <typename Type>
    struct Call {
        using FromType = propagateConst<Type, void>;

        static constexpr void call(FromType* from, void* to) noexcept(
            std::is_nothrow_move_assignable_v<Type>) {
            *static_cast<std::remove_cv_t<Type>*>(to) = std::move(*static_cast<Type*>(from));
        }
    };

    using Fn = void (*)(FromVoid*, void*);

    static constexpr Fn Array[sizeof...(Types)] = {
        Call<propagateConst<FromVoid, Types>>::call...,
    };
};

template <typename T>
concept SelfPtr = std::is_same_v<std::decay_t<T>, void>;

template <typename... Ts>
struct VTable {
    static constexpr void destroy(void* self, size_t index) {
        DestructorFunctorArray<Ts...>::call(self, index);
    }

    template <typename F, SelfPtr S>
    static constexpr decltype(auto) visit(F&& f, S* self, size_t index) {
        CallableFunctorArray<S, F, Ts...>::call(std::forward<F>(f), self, index);
    }

    template <typename Ref, SelfPtr S>
    static constexpr void construct(S* from, void* to, size_t index) {
        using Copy = CopyConstructorFunctorArray<S, Ts...>;
        using Move = MoveConstructorFunctorArray<S, Ts...>;

        RefSelector<Ref, Copy, Move>::call(from, to, index);
    }

    template <typename Ref, SelfPtr S>
    static constexpr void assign(S* from, void* to, size_t index) {
        using Copy = CopyAssignmentFunctorArray<S, Ts...>;
        using Move = MoveAssignmentFunctorArray<S, Ts...>;

        RefSelector<Ref, Copy, Move>::call(from, to, index);
    }
};

}  // namespace result::detail
