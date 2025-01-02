#pragma once

#include "result/detail/min_sized_type.h"
#include "result/detail/overloaded.h"
#include "result/detail/propagate_category.h"
#include "result/detail/strong_typedef.h"
#include "result/detail/vtable.h"

#include <type_list/list.h>

#include <algorithm>
#include <cstddef>
#include <cstring>

namespace result {

namespace detail {

struct Impossible {};

template <typename From, typename To>
concept ValueConvertibleTo =
    (std::is_convertible_v<typename From::ValueType, typename To::ValueType> ||
     std::is_same_v<typename From::ValueType, Impossible>);

template <typename From, typename To>
concept ErrorConvertibleTo = tl::SubsetOf<typename From::ErrorTypes, typename To::ErrorTypes>;

}  // namespace detail

template <typename From, typename To>
concept ConvertibleTo = detail::ValueConvertibleTo<std::decay_t<From>, std::decay_t<To>> &&
                        detail::ErrorConvertibleTo<std::decay_t<From>, std::decay_t<To>>;

template <typename E>
struct err_tag_t {};  // NOLINT

template <typename E>
constexpr err_tag_t<E> err_tag;  //  NOLINT

struct val_tag_t {};                 // NOLINT
inline constexpr val_tag_t val_tag;  // NOLINT

template <typename V, typename... Es>
class Result {
    static_assert(std::is_same_v<V, std::decay_t<V>>);
    static_assert((std::is_same_v<Es, std::decay_t<Es>> && ...));
    static_assert(tl::Set<tl::List<Es...>>);

    using Self = Result<V, Es...>;
    struct Val : detail::StrongTypedef<Val, V> {};

    using IndexType = detail::MinimalSizedIndexType<1 + sizeof...(Es)>;
    using Types = tl::List<Val, Es...>;
    using VTable = detail::VTable<Val, Es...>;

    static constexpr size_t StorageSize = std::max({sizeof(V), sizeof(Es)...});

 public:
    using value_type = V;  // NOLINT
    using ValueType = V;
    using ErrorTypes = tl::List<Es...>;

    template <typename U>
    using RebindValue = Result<U, Es...>;

    ~Result() noexcept {
        destroy();
    }

    template <std::same_as<V> U = V>
    requires std::is_default_constructible_v<U>
    Result() {
        new (ptr()) Val();
        set<Val>();
    }

    template <typename U = V>
    explicit(!std::is_convertible_v<U, V>) Result(U&& from) {
        new (ptr()) Val(std::forward<U>(from));
        set<Val>();
    }

    template <typename... Args>
    requires std::is_constructible_v<V, Args...>
    Result(std::in_place_t, Args&&... args) {
        new (ptr()) Val(std::forward<Args>(args)...);
        set<Val>();
    }

    template <typename... Args, std::constructible_from<Args...> E>
    requires tl::Contains<ErrorTypes, E>
    Result(err_tag_t<E>, Args&&... args) {
        new (ptr()) E(std::forward<Args>(args)...);
        set<E>();
    }

    Result(const Result& r) {
        construct(r);
    }

    Result(Result&& r) noexcept {
        construct(std::move(r));
    }

    template <ConvertibleTo<Self> R>
    Result(R&& from) {
        construct(std::forward<R>(from));
    }

    Result& operator=(Result& from) {  // NOLINT
        assign(from);
        return *this;
    }

    Result& operator=(const Result& from) {
        assign(from);
        return *this;
    }

    Result& operator=(Result&& from) noexcept {
        assign(std::move(from));
        return *this;
    }

    template <ConvertibleTo<Self> R>
    Result& operator=(R&& from) {
        assign(std::forward<R>(from));
        return *this;
    }

    template <typename F, typename Self>
    decltype(auto) visit(this Self&& self, F&& f) {  // NOLINT
        using RVal = detail::propagateConst<Self, Val>;

        return VTable::dispatch(
            detail::Overloaded{
                [&](RVal& value) { return f(std::forward_like<Self>(value).get()); },
                [&]<typename E>(E& error) { return f(std::forward_like<Self>(error)); },
            },
            self.ptr(),
            self.index());
    }

    template <typename F, typename Self>
    decltype(auto) taggedVisit(this Self&& self, F&& f) {  // NOLINT
        using RVal = detail::propagateConst<Self, Val>;

        return VTable::dispatch(
            detail::Overloaded{
                [&](RVal& value) { return f(val_tag, std::forward_like<Self>(value).get()); },
                [&]<typename E>(E& error) { return f(std::forward_like<Self>(error)); },
            },
            self.ptr(),
            self.index());
    }

    template <typename Self>
    decltype(auto) operator*(this Self&& self) {
        return std::forward<Self>(self).value();
    }

    template <typename Self>
    decltype(auto) operator->(this Self& self) {
        return &self.value();
    }

    template <typename U, typename Self>
    [[nodiscard]] V valueOr(this Self&& self, U&& default_value) {
        return self.hasValue() ? std::forward<Self>(self).value()
                               : static_cast<V>(std::forward<U>(default_value));
    }

    template <typename Self>
    [[nodiscard]] decltype(auto) value(this Self&& self) {
        return std::forward<Self>(self).template as<Val>().get();
    }

    template <typename E, typename Self>
    requires tl::Contains<ErrorTypes, E>
    [[nodiscard]] decltype(auto) error(this Self&& self) {
        return std::forward<Self>(self).template as<E>();
    }

    [[nodiscard]] bool hasValue() const noexcept {
        return is<Val>();
    }

    [[nodiscard]] bool hasAnyError() const noexcept {
        return !hasValue();
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return !hasAnyError();
    }

    template <typename E>
    requires tl::Contains<ErrorTypes, E>
    [[nodiscard]] bool hasError() const noexcept {
        return is<E>();
    }

    [[nodiscard]] size_t index() const noexcept {
        return index_;
    }

    [[nodiscard]] /*static*/ constexpr size_t valueIndex() const noexcept {
        return tl::Find<Val, Types>;
    }

    template <typename E>
    requires tl::Contains<ErrorTypes, E>
    [[nodiscard]] /*static*/ constexpr size_t errorIndex() const noexcept {
        return tl::Find<E, Types>;
    }

 private:
    template <ConvertibleTo<Self> R>
    void construct(R&& from) {  // NOLINT
        using From = std::decay_t<R>;
        using FromVTable = typename From::VTable;
        using FromVal = detail::propagateConst<R, typename From::Val>;

        FromVTable::dispatch(
            detail::Overloaded{
                [&](FromVal& val) {  //
                    using FV = typename From::value_type;

                    if constexpr (!std::is_same_v<FV, detail::Impossible>) {
                        new (this) Result(std::forward_like<R>(val.get()));
                    } else {
                        std::unreachable();
                    }
                },
                [&]<typename G>(G& err) {
                    using E = std::decay_t<G>;
                    new (this) Result(err_tag<E>, std::forward_like<R>(err));
                },
            },
            from.ptr(),
            from.index());
    }

    template <ConvertibleTo<Self> R>
    void assign(R&& from) {  // NOLINT
        if constexpr (std::is_same_v<Self, std::decay_t<R>>) {
            if (this == &from) [[unlikely]] {
                return;
            }
        }

        using From = std::decay_t<R>;
        using FromVal = detail::propagateConst<R, typename From::Val>;

        From::VTable::dispatch(
            detail::Overloaded{
                [&](FromVal& val) {  //
                    using FV = typename From::value_type;

                    if constexpr (!std::is_same_v<FV, detail::Impossible>) {
                        if constexpr (std::is_same_v<FV, value_type>) {
                            if (is<Val>()) {
                                value() = std::forward_like<R>(val.get());
                                return;
                            }
                        }

                        destroy();
                        new (this) Result(std::forward_like<R>(val.get()));
                    } else {
                        std::unreachable();
                    }
                },
                [&]<typename G>(G& err) {
                    using E = std::decay_t<G>;

                    if (is<E>()) {
                        error<E>() = std::forward_like<R>(err);
                    } else {
                        destroy();
                        new (this) Result(err_tag<E>, std::forward_like<R>(err));
                    }
                },
            },
            from.ptr(),
            from.index());
    }

    template <typename T>
    requires tl::Contains<Types, T>
    void set() {
        index_ = tl::Find<T, Types>;
    }

    template <typename T>
    requires tl::Contains<Types, T>
    bool is() const noexcept {
        return index_ == tl::Find<T, Types>;
    }

    template <typename T, typename Self>
    requires tl::Contains<Types, T>
    decltype(auto) as(this Self&& self) noexcept {
        using U = detail::propagateCategory<Self&&, T>;
        return reinterpret_cast<U>(std::forward<Self>(self).data_);  // NOLINT
    }

 private:
    void destroy() noexcept {
        VTable::dispatch(
            []<typename T>(T& value) {
                using U = std::decay_t<T>;
                value.~U();
            },
            ptr(),
            index());
    }

    void* ptr() noexcept {
        return &data_;
    }

    const void* ptr() const noexcept {
        return &data_;
    }

    alignas(V) alignas(Es...) std::byte data_[StorageSize]{};
    IndexType index_;

    template <typename U, typename... Gs>
    friend class Result;
};

struct Unit {};

inline constexpr Unit unit{};  // NOLINT

template <typename... Es>
using Status = Result<Unit, Es...>;

template <typename E>
Result<detail::Impossible, std::decay_t<E>> makeError(E&& error) {
    using G = std::decay_t<E>;
    return Result<detail::Impossible, G>(err_tag<G>, std::forward<E>(error));
}

template <typename E, typename... Args>
Result<detail::Impossible, std::decay_t<E>> makeError(Args&&... args) {
    return Result<detail::Impossible, E>(err_tag<E>, std::forward<Args>(args)...);
}

}  // namespace result
