#pragma once

#include "result/detail/min_sized_type.h"
#include "result/detail/overloaded.h"
#include "result/detail/propagate_category.h"
#include "result/detail/vtables.h"

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

struct Value {};

}  // namespace detail

template <typename From, typename To>
concept ConvertibleTo = detail::ValueConvertibleTo<std::decay_t<From>, std::decay_t<To>> &&
                        detail::ErrorConvertibleTo<std::decay_t<From>, std::decay_t<To>>;

template <typename E>
struct err_tag_t {};  // NOLINT

template <typename E>
constexpr err_tag_t<E> err_tag;  //  NOLINT

// For Result::visit
struct val_tag_t {};                 // NOLINT
inline constexpr val_tag_t val_tag;  // NOLINT

template <typename V, typename... Es>
class Result {
    static_assert(std::is_same_v<V, std::decay_t<V>>);
    static_assert((std::is_same_v<Es, std::decay_t<Es>> && ...));
    static_assert(tl::Set<tl::List<Es...>>);

    using Self = Result<V, Es...>;
    using VTable = detail::VTable<V, Es...>;
    using StoredTypes = tl::List<V, Es...>;
    using Types = tl::List<detail::Value, Es...>;
    using IndexType = detail::MinimalSizedIndexType<1 + sizeof...(Es)>;

    static constexpr size_t StorageSize = std::max({sizeof(V), sizeof(Es)...});
    static constexpr bool ValueInErrors = tl::Contains<tl::List<Es...>, V>;

 public:
    using value_type = V;  // NOLINT
    using ValueType = V;
    using ErrorTypes = tl::List<Es...>;

    template <typename U>
    using RebindValue = Result<U, Es...>;

    ~Result() noexcept {
        VTable::destroy(ptr(), index_);
    }

    template <std::same_as<V> U = V>
    requires std::is_default_constructible_v<U>
    Result() {
        new (ptr()) V();
        set<detail::Value>();
    }

    template <typename U = V>
    explicit(!std::is_convertible_v<U, V>) Result(U&& from) {
        new (ptr()) V(std::forward<U>(from));
        set<detail::Value>();
    }

    template <typename... Args>
    requires std::is_constructible_v<V, Args...>
    Result(std::in_place_t, Args&&... args) {
        new (ptr()) V(std::forward<Args>(args)...);
        set<detail::Value>();
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
        return VTable::visit(std::forward<F>(f), self.ptr(), self.index_);
    }

    template <typename F, typename Self>
    decltype(auto) safeVisit(this Self&& self, F&& f) {  // NOLINT
        return std::forward<Self>(self).visit([&]<typename U>(U& value) {
            using T = std::decay_t<U>;
            constexpr bool is_value = std::is_same_v<T, V>;

            if constexpr (is_value) {
                if constexpr (ValueInErrors) {
                    if (self.index() == self.valueIndex()) {
                        return f(val_tag, std::forward_like<Self>(value));
                    } else {
                        return f(std::forward_like<Self>(value));
                    }
                } else {
                    return f(val_tag, std::forward_like<Self>(value));
                }
            } else {
                return f(std::forward_like<Self>(value));
            }
        });
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
        return std::forward<Self>(self).template as<V>();
    }

    template <typename E, typename Self>
    requires tl::Contains<ErrorTypes, E>
    [[nodiscard]] decltype(auto) error(this Self&& self) {
        return std::forward<Self>(self).template as<E>();
    }

    [[nodiscard]] bool hasValue() const noexcept {
        return is<detail::Value>();
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
        return tl::Find<detail::Value, Types>;
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

        if constexpr (
            !std::is_same_v<detail::Impossible, typename From::value_type> &&
            !std::is_same_v<value_type, typename From::value_type>) {
            if (from.hasValue()) {
                // from has value that is convertible to our value
                new (ptr()) V(std::forward<R>(from).value());
                set<detail::Value>();
                return;
            }
        }

        const auto from_index = from.index_;
        FromVTable::template construct<decltype(from)>(from.ptr(), ptr(), from.index_);

        static constexpr auto index_map = tl::injection(typename From::Types{}, Types{});
        index_ = index_map[from_index];
    }

    template <ConvertibleTo<Self> R>
    void assign(R&& from) {
        if constexpr (std::is_same_v<Self, std::decay_t<R>>) {
            if (this == &from) [[unlikely]] {
                return;
            }
        }

        using From = std::decay_t<R>;
        using FromVTable = typename From::VTable;

        if constexpr (
            !std::is_same_v<detail::Impossible, typename From::value_type> &&
            !std::is_same_v<value_type, typename From::value_type>) {
            if (from.hasValue()) {
                // from has value of different type. need to destroy anyhow
                VTable::destroy(ptr(), index_);
                new (this) Result(std::forward<R>(from));
                return;
            }
        }

        static constexpr auto index_map = tl::injection(typename From::Types{}, Types{});
        const auto from_index = from.index_;
        const auto this_index = index_map[from_index];

        if (index_ == this_index) {
            FromVTable::template assign<decltype(from)>(from.ptr(), ptr(), from_index);
        } else {
            VTable::destroy(ptr(), index_);
            new (this) Result(std::forward<R>(from));
        }
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
    requires tl::Contains<StoredTypes, T>
    decltype(auto) as(this Self&& self) noexcept {
        using U = detail::propagateCategory<Self&&, T>;
        return reinterpret_cast<U>(std::forward<Self>(self).data_);  // NOLINT
    }

 private:
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
