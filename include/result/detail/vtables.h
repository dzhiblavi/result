#pragma once

#include "result/detail/helpers.h"
#include "result/detail/propagate_const.h"

#include <type_traits>

namespace result::vtables {

template <bool IsTriviallyDestructible, typename Type>
struct DestructorFunctor {
    static constexpr void Call(void* ptr) noexcept {
        static_cast<Type*>(ptr)->~Type();
    }
};

template <typename Type>
struct DestructorFunctor<true, Type> {
    static constexpr void Call(void*) noexcept {}
};

template <typename... Types>
struct DestructorFunctorArray {
    using DestructorFunction = void (*)(void*) noexcept;

    static constexpr DestructorFunction array[sizeof...(Types)] = {
        DestructorFunctor<std::is_trivially_destructible_v<Types>, Types>::Call...,
    };

    static constexpr void Call(void* ptr, size_t index) noexcept {
        array[index](ptr);
    }
};

template <typename Callable, typename Type>
struct CallableFunctor {
    using PointerType = util::hlp::propagateConst<Type, void>;

    static constexpr decltype(auto) Call(Callable callable, PointerType* ptr) {
        return std::forward<Callable>(callable)(*static_cast<Type*>(ptr));
    }
};

template <typename FromVoid, typename Callable, typename... Types>
struct CallableFunctorArray {
    using ResultType = detail::VisitInvokeResult<Callable, Types...>;
    using CallFunction = ResultType (*)(Callable, void*);

    static constexpr CallFunction array[sizeof...(Types)] = {
        CallableFunctor<Callable, util::hlp::propagateConst<FromVoid, Types>>::Call...};

    template <typename F>
    static constexpr void Call(F&& func, FromVoid* ptr, size_t index) {
        array[index](std::forward<F>(func), ptr);
    }
};

template <typename Type>
struct CopyConstructorFunctor {
    using FromType = util::hlp::propagateConst<Type, void>;

    static constexpr void Call(FromType* from, void* to) noexcept(
        std::is_nothrow_copy_constructible_v<Type>) {
        if constexpr (!std::is_same_v<void, std::decay_t<Type>>) {
            new (to) std::remove_cv_t<Type>(*static_cast<Type*>(from));
        }
    }
};

template <typename FromVoid, typename... Types>
struct CopyConstructorFunctorArray {
    using CopyConstructorFunction = void (*)(FromVoid*, void*);

    static constexpr CopyConstructorFunction array[sizeof...(Types)] = {
        CopyConstructorFunctor<util::hlp::propagateConst<FromVoid, Types>>::Call...};

    static constexpr void Call(FromVoid* from, void* to, size_t index) {
        array[index](from, to);
    }
};

template <typename Type>
struct CopyAssignmentFunctor {
    using FromType = util::hlp::propagateConst<Type, void>;

    static constexpr void Call(FromType* from, void* to) noexcept(
        std::is_nothrow_copy_assignable_v<Type>) {
        if constexpr (!std::is_same_v<void, std::decay_t<Type>>) {
            *static_cast<std::remove_cv_t<Type>*>(to) = *static_cast<Type*>(from);
        }
    }
};

template <typename FromVoid, typename... Types>
struct CopyAssignmentFunctorArray {
    using CopyAssignmentFunction = void (*)(FromVoid*, void*);

    static constexpr CopyAssignmentFunction array[sizeof...(Types)] = {
        CopyAssignmentFunctor<util::hlp::propagateConst<FromVoid, Types>>::Call...};

    static constexpr void Call(FromVoid* from, void* to, size_t index) {
        array[index](from, to);
    }
};

template <typename Type>
struct MoveConstructorFunctor {
    using FromType = util::hlp::propagateConst<Type, void>;

    static constexpr void Call(FromType* from, void* to) noexcept(
        std::is_nothrow_move_constructible_v<Type>) {
        if constexpr (!std::is_same_v<void, std::decay_t<Type>>) {
            new (to) std::remove_cv_t<Type>(std::move(*static_cast<Type*>(from)));
        }
    }
};

template <typename FromVoid, typename... Types>
struct MoveConstructorFunctorArray {
    using MoveConstructorFunction = void (*)(FromVoid*, void*);

    static constexpr MoveConstructorFunction array[sizeof...(Types)] = {
        MoveConstructorFunctor<util::hlp::propagateConst<FromVoid, Types>>::Call...};

    static constexpr void Call(FromVoid* from, void* to, size_t index) {
        array[index](from, to);
    }
};

template <typename Type>
struct MoveAssignmentFunctor {
    using FromType = util::hlp::propagateConst<Type, void>;

    static constexpr void Call(FromType* from, void* to) noexcept(
        std::is_nothrow_move_assignable_v<Type>) {
        if constexpr (!std::is_same_v<void, std::decay_t<Type>>) {
            *static_cast<std::remove_cv_t<Type>*>(to) = std::move(*static_cast<Type*>(from));
        }
    }
};

template <typename FromVoid, typename... Types>
struct MoveAssignmentFunctorArray {
    using MoveAssignmentFunction = void (*)(FromVoid*, void*);

    static constexpr MoveAssignmentFunction array[sizeof...(Types)] = {
        MoveAssignmentFunctor<util::hlp::propagateConst<FromVoid, Types>>::Call...};

    static constexpr void Call(FromVoid* from, void* to, size_t index) {
        array[index](from, to);
    }
};

}  // namespace result::vtables
