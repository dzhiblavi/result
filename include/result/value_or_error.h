#pragma once

#include "result/detail/assert.h"
#include "result/detail/helpers.h"
#include "result/detail/vtables.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <type_traits>

namespace result {

namespace detail {

template <typename... Types>
struct VariantStorage {
    static constexpr size_t StorageSize = std::max({0ul, sizeof(Types)...});
    using IndexType = MinimalSizedIndexType<1 + sizeof...(Types)>;

    alignas(Types...) std::byte data[StorageSize];
    IndexType index{0};
};

template <typename Type>
struct ValueTypeWrapper {};

template <typename... Stored>
struct TraitsBase : public VariantStorage<Stored...> {
 public:
    using StorageType = VariantStorage<Stored...>;
    using IndexType = typename StorageType::IndexType;

    using DestructorArray = vtables::DestructorFunctorArray<Stored...>;

    template <typename FromVoid>
    using CopyConstructorArray = vtables::CopyConstructorFunctorArray<FromVoid, Stored...>;

    template <typename FromVoid>
    using MoveConstructorArray = vtables::MoveConstructorFunctorArray<FromVoid, Stored...>;

    template <typename FromVoid>
    using CopyAssignmentArray = vtables::CopyAssignmentFunctorArray<FromVoid, Stored...>;

    template <typename FromVoid>
    using MoveAssignmentArray = vtables::MoveAssignmentFunctorArray<FromVoid, Stored...>;

    template <typename Callable, typename FromVoid>
    using VisitArray = vtables::CallableFunctorArray<FromVoid, Callable, Stored...>;

    template <typename Ref, typename FromVoid>
    using ConstructorRefSelector =
        ReferenceSelector<Ref, CopyConstructorArray, MoveConstructorArray, FromVoid>;

    template <typename Ref, typename FromVoid>
    using AssignmentRefSelector =
        ReferenceSelector<Ref, CopyAssignmentArray, MoveAssignmentArray, FromVoid>;

    const IndexType& LogicalIndex() const noexcept {
        return StorageType::index;
    }
    IndexType& LogicalIndex() noexcept {
        return StorageType::index;
    }
    IndexType PhysicalIndex() const noexcept {
        return LogicalToPhysicalIndex(LogicalIndex());
    }

    const auto& Data() const noexcept {
        return StorageType::data;
    }
    auto& Data() noexcept {
        return StorageType::data;
    }

    static constexpr IndexType LogicalEmptyIndex() noexcept {
        return 0;
    }
    static constexpr IndexType LogicalToPhysicalIndex(IndexType index) noexcept {
        return index - 1;
    }
    static constexpr IndexType PhysicalToLogicalIndex(IndexType index) noexcept {
        return index + 1;
    }

    /**
     * @return whether this object neither holds a value nor an error (is empty)
     */
    constexpr bool IsEmpty() const noexcept {
        return LogicalIndex() == LogicalEmptyIndex();
    }

 private:
    using StorageType::data;
    using StorageType::index;
};

template <typename... Types>
struct Traits;

template <typename ValueType, typename... ErrorTypes>
struct Traits<ValueType, ErrorTypes...> : public TraitsBase<ValueType, ErrorTypes...> {
    using Base = TraitsBase<ValueType, ErrorTypes...>;
    using StoredTypesList = list::list<ValueTypeWrapper<ValueType>, ErrorTypes...>;
    using ErrorTypesList = list::list<ErrorTypes...>;

    static constexpr size_t LogicalValueIndex() noexcept {
        return 1;
    }
    static constexpr size_t LogicalFirstErrorIndex() noexcept {
        return 2;
    }

    template <
        typename ErrorType,
        typename = std::enable_if_t<list::contains<ErrorTypesList, ErrorType>>>
    static constexpr size_t LogicalErrorIndex() noexcept {
        return LogicalFirstErrorIndex() + list::indexOf<ErrorTypesList, ErrorType>;
    }
};

template <typename... ErrorTypes>
struct Traits<void, ErrorTypes...> : public TraitsBase<ErrorTypes...> {
    using Base = TraitsBase<ErrorTypes...>;
    using StoredTypesList = list::list<ErrorTypes...>;
    using ErrorTypesList = list::list<ErrorTypes...>;

    static constexpr size_t LogicalFirstErrorIndex() noexcept {
        return 1;
    }

    template <
        typename ErrorType,
        typename = std::enable_if_t<list::contains<ErrorTypesList, ErrorType>>>
    static constexpr size_t LogicalErrorIndex() noexcept {
        return LogicalFirstErrorIndex() + list::indexOf<ErrorTypesList, ErrorType>;
    }
};

template <bool AllTriviallyDestructible, typename... Types>
struct DestructorHolder : public Traits<Types...> {
    using Base = Traits<Types...>;

    ~DestructorHolder() noexcept {
        DestroyImpl();
    }

    /**
     * @brief Clears the object. The state after method is applied is Empty.
     */
    void Clear() noexcept {
        DestroyImpl();
        Base::LogicalIndex() = Base::LogicalEmptyIndex();
    }

 private:
    void DestroyImpl() noexcept {
        if (Base::IsEmpty()) [[unlikely]] {
            return;
        }

        Base::DestructorArray::Call(Base::Data(), Base::PhysicalIndex());
    }
};

template <typename... Types>
struct DestructorHolder<true, Types...> : public Traits<Types...> {
    using Base = Traits<Types...>;

    ~DestructorHolder() noexcept = default;

    /**
     * @brief Clears the object. The state after method is applied is Empty.
     */
    void Clear() noexcept {
        Base::LogicalIndex() = Base::LogicalEmptyIndex();
    }
};

template <typename... Types>
using DestructorImpl = DestructorHolder<
    (... && (std::is_trivially_destructible_v<Types> || std::is_same_v<void, Types>)),
    Types...>;

template <typename ValueType, typename... ErrorTypes>
struct GetValueImpl : public DestructorImpl<ValueType, ErrorTypes...> {
    using Base = DestructorImpl<ValueType, ErrorTypes...>;

    /**
     * @return whether the object holds a value.
     */
    constexpr bool HasValue() const noexcept {
        return Base::LogicalIndex() == Base::LogicalValueIndex();
    }

    /**
     * @return a reference to underlying value
     * @exception UB is HasValue() == false
     */
    ValueType& GetValue() & noexcept {
        RESULT_ASSERT(HasValue(), "GetValue() called on object with no value");
        return reinterpret_cast<ValueType&>(Base::Data());
    }

    /**
     * @return moved underlying value
     * @exception UB is HasValue() == false
     */
    ValueType&& GetValue() && noexcept {
        RESULT_ASSERT(HasValue(), "GetValue() called on object with no value");
        return reinterpret_cast<ValueType&&>(Base::Data());
    }

    /**
     * @return a const reference to underlying value
     * @exception UB is HasValue() == false
     */
    const ValueType& GetValue() const& noexcept {
        RESULT_ASSERT(HasValue(), "GetValue() called on object with no value");
        return reinterpret_cast<const ValueType&>(Base::Data());
    }
};

template <typename... ErrorTypes>
struct GetValueImpl<void, ErrorTypes...> : public DestructorImpl<void, ErrorTypes...> {
    using Base = DestructorImpl<void, ErrorTypes...>;

    /**
     * @return false
     * @note Result<void, ...> never holds a value
     */
    constexpr bool HasValue() const noexcept {
        return false;
    }
};

template <typename ValueType, typename... ErrorTypes>
struct GetErrorImpl : public GetValueImpl<ValueType, ErrorTypes...> {
    using Base = GetValueImpl<ValueType, ErrorTypes...>;
    using typename Base::ErrorTypesList;

    /**
     * @brief Get Error type by its index in ErrorTypes... list
     */
    template <size_t Index>
    using ErrorType = list::get<ErrorTypesList, Index>;

    /**
     * @return whether the object holds any error
     */
    constexpr bool HasAnyError() const noexcept {
        return Base::LogicalIndex() >= Base::LogicalFirstErrorIndex();
    }

    /**
     * @return the index of underlying error in ErrorTypes... list
     * @exception UB if !HasAnyError()
     */
    size_t GetErrorIndex() const noexcept {
        RESULT_ASSERT(HasAnyError(), "GetErrorIndex() called on object with no error");
        return Base::LogicalIndex() - Base::LogicalFirstErrorIndex();
    }

    /**
     * @return whether this object holds an error with the specified type
     */
    template <
        typename ErrorType,
        typename = std::enable_if_t<list::contains<ErrorTypesList, ErrorType>>>
    bool HasError() const noexcept {
        return Base::LogicalIndex() == Base::template LogicalErrorIndex<ErrorType>();
    }

    /**
     * @return reference to underlying error of the specified type
     * @exception UB if !HasError<ErrorType>()
     */
    template <
        typename ErrorType,
        typename = std::enable_if_t<list::contains<ErrorTypesList, ErrorType>>>
    ErrorType& GetError() & noexcept {
        RESULT_ASSERT(HasError<ErrorType>(), "GetError<E>() called on object with no error E");
        return reinterpret_cast<ErrorType&>(Base::Data());
    }

    /**
     * @return rvalue reference to the underlying error object of the specified type
     * @exception UB if !HasError<ErrorType>()
     */
    template <
        typename ErrorType,
        typename = std::enable_if_t<list::contains<ErrorTypesList, ErrorType>>>
    ErrorType&& GetError() && noexcept {
        RESULT_ASSERT(HasError<ErrorType>(), "GetError<E>() called on object with no error E");
        return reinterpret_cast<ErrorType&&>(Base::Data());
    }

    /**
     * @return const reference to underlying error object of the specified type
     * @exception UB if !HasError<ErrorType>()
     */
    template <
        typename ErrorType,
        typename = std::enable_if_t<list::contains<ErrorTypesList, ErrorType>>>
    const ErrorType& GetError() const& noexcept {
        RESULT_ASSERT(HasError<ErrorType>(), "GetError<E>() called on object with no error E");
        return reinterpret_cast<const ErrorType&>(Base::Data());
    }

    /**
     * @return const rvalue reference to underlying error object of the specified type
     * @exception UB if !HasError<ErrorType>()
     */
    template <
        typename ErrorType,
        typename = std::enable_if_t<list::contains<ErrorTypesList, ErrorType>>>
    const ErrorType&& GetError() const&& noexcept {
        RESULT_ASSERT(HasError<ErrorType>(), "GetError<E>() called on object with no error E");
        return reinterpret_cast<const ErrorType&&>(Base::Data());
    }

    /**
     * @return reference to underlying error with type ErrorType<Index>
     * @exception UB if !HasError<ErrorType<Index>>()
     */
    template <size_t Index>
    ErrorType<Index>& GetError() & noexcept {
        RESULT_ASSERT(
            HasError<ErrorType<Index>>(), "GetError<I>() called on object with no error E[I]");
        return reinterpret_cast<ErrorType<Index>&>(Base::Data());
    }

    /**
     * @return rvalue reference to underlying error with type ErrorType<Index>
     * @exception UB if !HasError<ErrorType<Index>>()
     */
    template <size_t Index>
    ErrorType<Index>&& GetError() && noexcept {
        RESULT_ASSERT(
            HasError<ErrorType<Index>>(), "GetError<I>() called on object with no error E[I]");
        return reinterpret_cast<ErrorType<Index>&>(Base::Data());
    }

    /**
     * @return const reference to underlying error with type ErrorType<Index>
     * @exception UB if !HasError<ErrorType<Index>>()
     */
    template <size_t Index>
    const ErrorType<Index>& GetError() const& noexcept {
        RESULT_ASSERT(
            HasError<ErrorType<Index>>(), "GetError<I>() called on object with no error E[I]");
        return reinterpret_cast<const ErrorType<Index>&>(Base::Data());
    }

    /**
     * @return const rvalue reference to underlying error with type ErrorType<Index>
     * @exception UB if !HasError<ErrorType<Index>>()
     */
    template <size_t Index>
    const ErrorType<Index>&& GetError() const&& noexcept {
        RESULT_ASSERT(
            HasError<ErrorType<Index>>(), "GetError<I>() called on object with no error E[I]");
        return reinterpret_cast<const ErrorType<Index>&&>(Base::Data());
    }
};

template <typename ValueType, typename... ErrorTypes>
struct SetErrorImpl : public GetErrorImpl<ValueType, ErrorTypes...> {
    using Base = GetErrorImpl<ValueType, ErrorTypes...>;
    using typename Base::ErrorTypesList;

    /**
     * @brief Sets the error with the specified value
     */
    template <
        typename ErrorType,
        typename Decayed = std::decay_t<ErrorType>,
        typename = std::enable_if_t<list::contains<ErrorTypesList, Decayed>>>
    void SetError(ErrorType&& error) & {
        Base::Clear();
        Base::LogicalIndex() = Base::template LogicalErrorIndex<Decayed>();
        new (Base::Data()) Decayed(std::forward<ErrorType>(error));
    }

    /**
     * @brief Sets the error by constructing it inplace via forwarding constructor arguments
     */
    template <typename ErrorType, typename... Args>
    void EmplaceError(Args&&... args) & {
        Base::Clear();
        Base::LogicalIndex() = Base::template LogicalErrorIndex<ErrorType>();
        new (Base::Data()) ErrorType(std::forward<Args>(args)...);
    }
};

template <typename ValueType, typename... ErrorTypes>
struct ValueConstructorImpl : public SetErrorImpl<ValueType, ErrorTypes...> {
    using Base = SetErrorImpl<ValueType, ErrorTypes...>;

    /**
     * @brief Constructs the object as holing a value using the specified value object
     */
    template <
        typename FromType,
        typename = std::enable_if_t<std::is_same_v<ValueType, std::decay_t<FromType>>>>
    void ValueConstruct(FromType&& from) noexcept(std::is_nothrow_copy_constructible_v<ValueType>) {
        Base::LogicalIndex() = Base::LogicalValueIndex();
        new (Base::Data()) ValueType(std::forward<FromType>(from));
    }
};

template <typename ValueType, typename... ErrorTypes>
struct ConstructorsImpl : public ValueConstructorImpl<ValueType, ErrorTypes...> {
    using Base = ValueConstructorImpl<ValueType, ErrorTypes...>;

 protected:
    template <typename From>
    void Construct(From&& from) {
        if (from.IsEmpty()) [[unlikely]] {
            return;
        }

        Base::LogicalIndex() = from.LogicalIndex();
        Base ::
            template ConstructorRefSelector<decltype(from), util::hlp::propagateConst<From, void>>::
                Call(from.Data(), Base::Data(), Base::PhysicalIndex());
    }

    template <typename From, typename FromValueType, typename... FromErrorTypes>
    void ConvertConstruct(From&& from, ConstructorsImpl<FromValueType, FromErrorTypes...>*) {
        if (from.IsEmpty()) [[unlikely]] {
            return;
        }

        using FromType = ConstructorsImpl<FromValueType, FromErrorTypes...>;
        using PhysicalIndexMapping = typename IndexMapping<
            typename FromType::StoredTypesList>::template MapTo<typename Base::StoredTypesList>;

        const size_t this_phys_index = PhysicalIndexMapping::map(from.PhysicalIndex());
        RESULT_ASSERT(
            this_phys_index != size_t(-1),
            "Conversion constructor from Result<X, ...> to Result<void, ...>"
            " is trying to drop a value");

        FromType::
            template ConstructorRefSelector<decltype(from), util::hlp::propagateConst<From, void>>::
                Call(from.Data(), Base::Data(), from.PhysicalIndex());
        Base::LogicalIndex() = Base::PhysicalToLogicalIndex(this_phys_index);
    }
};

template <typename ValueType, typename... ErrorTypes>
struct AssignmentsImpl : public ConstructorsImpl<ValueType, ErrorTypes...> {
    using Base = ConstructorsImpl<ValueType, ErrorTypes...>;

    template <
        typename Assignee,
        typename = std::enable_if_t<
            std::is_same_v<std::decay_t<Assignee>, Result<ValueType, ErrorTypes...>>>>
    void Assign(Assignee&& rhs) & noexcept {
        if (this == &rhs) [[unlikely]] {
            return;
        }

        if (rhs.IsEmpty()) {
            Base::Clear();
            return;
        }

        if (Base::LogicalIndex() == rhs.LogicalIndex()) {
            Base::template AssignmentRefSelector<
                decltype(rhs),
                util::hlp::propagateConst<Assignee, void>>::
                Call(rhs.Data(), Base::Data(), Base::PhysicalIndex());
            return;
        }

        Base::Clear();
        Base::LogicalIndex() = rhs.LogicalIndex();
        Base::template ConstructorRefSelector<
            decltype(rhs),
            util::hlp::propagateConst<Assignee, void>>::
            Call(rhs.Data(), Base::Data(), Base::PhysicalIndex());
    }

    template <typename Convert, typename FromValueType, typename... FromErrorTypes>
    void ConvertAssign(Convert&& rhs, Result<FromValueType, FromErrorTypes...>*) & noexcept {
        if (rhs.IsEmpty()) [[unlikely]] {
            Base::Clear();
            return;
        }

        using RhsType = Result<FromValueType, FromErrorTypes...>;
        using PhysicalIndexMapping = typename IndexMapping<
            typename RhsType::StoredTypesList>::template MapTo<typename Base::StoredTypesList>;

        const size_t rhs_phys_index = rhs.PhysicalIndex();
        const size_t this_phys_index = PhysicalIndexMapping::map(rhs_phys_index);

        RESULT_ASSERT(
            this_phys_index != size_t(-1),
            "Conversion assignment of Result<X, ...> to Result<void, ...>"
            " is trying to drop a value");

        if (Base::PhysicalIndex() == this_phys_index) {
            RhsType::template AssignmentRefSelector<
                decltype(rhs),
                util::hlp::propagateConst<Convert, void>>::
                Call(rhs.Data(), Base::Data(), rhs_phys_index);
            return;
        }

        Base::Clear();
        Base::LogicalIndex() = Base::PhysicalToLogicalIndex(this_phys_index);
        RhsType::template ConstructorRefSelector<
            decltype(rhs),
            util::hlp::propagateConst<Convert, void>>::
            Call(rhs.Data(), Base::Data(), rhs_phys_index);
    }
};

template <typename ValueType, typename... ErrorTypes>
struct DiscardErrorImpl : public AssignmentsImpl<ValueType, ErrorTypes...> {
    using Base = AssignmentsImpl<ValueType, ErrorTypes...>;
    using typename Base::ErrorTypesList;

    template <typename... DiscardedErrors>
    using ResultType = TransferTemplate<
        list::set::subtract<ErrorTypesList, list::list<DiscardedErrors...>>,
        Result,
        ValueType>;

    /**
     * @brief Transforms the type by removing the specified error types
     *
     * In all scenarios, this object will become empty as the value or error will be moved out.
     * - If the object was empty, the result is empty.
     * - If the object held a value, the result holds the moved value.
     * - If the object held an error, the result holds it in case the resulting type has this error.
     * - If the object held an error which type is being removed, the result is empty.
     */
    template <typename... DiscardedErrors>
    ResultType<DiscardedErrors...> DiscardErrors() {
        using ResultT = ResultType<DiscardedErrors...>;
        ResultT result;

        if (Base::IsEmpty()) [[unlikely]] {
            return result;
        }

        using PhysicalIndexMapping = typename IndexMapping<
            typename Base::StoredTypesList>::template MapTo<typename ResultT::StoredTypesList>;

        const size_t result_phys_index = PhysicalIndexMapping::map(Base::PhysicalIndex());
        if (result_phys_index == size_t(-1)) {
            return result;
        }

        Base::template MoveConstructorArray<void>::Call(
            Base::Data(), result.Data(), Base::PhysicalIndex());
        Base::Clear();

        result.LogicalIndex() = ResultT::PhysicalToLogicalIndex(result_phys_index);
        return result;
    }
};

template <typename ValueType, typename... ErrorTypes>
struct DiscardValueImpl : public DiscardErrorImpl<ValueType, ErrorTypes...> {
    using Base = DiscardErrorImpl<ValueType, ErrorTypes...>;

    /**
     * @brief Discards the value from the type
     * @return an object of type Result<void, ErrorTypes...> with the same state as this
     * @exception UB: this object holds a value
     */
    Result<void, ErrorTypes...> DiscardValue() && noexcept {
        RESULT_ASSERT(!Base::HasValue(), "Discarding ValueType on object holding a value");
        return Result<void, ErrorTypes...>(std::move(*this));
    }
};

template <typename... ErrorTypes>
struct DiscardValueImpl<void, ErrorTypes...> : public DiscardErrorImpl<void, ErrorTypes...> {
    using Base = DiscardErrorImpl<void, ErrorTypes...>;

    /**
     * @brief Discards the value from the type
     * @return an object of type Result<void, ErrorTypes...> with the same state as this
     * @note actually does nothing for VoidOrError instances
     */
    auto& DiscardValue() & noexcept {
        return *this;
    }

    /**
     * @brief Discards the value from the type
     * @return an object of type Result<void, ErrorTypes...> with the same state as this
     * @note actually does nothing for VoidOrError instances
     */
    const auto& DiscardValue() const& noexcept {
        return *this;
    }

    /**
     * @brief Discards the value from the type
     * @return an object of type Result<void, ErrorTypes...> with the same state as this
     * @note moves out the object
     */
    auto DiscardValue() && noexcept {
        return std::move(*this);
    }
};

template <typename ValueType, typename... ErrorTypes>
struct VisitImpl : public DiscardValueImpl<ValueType, ErrorTypes...> {
    using Base = DiscardValueImpl<ValueType, ErrorTypes...>;

    /**
     * @brief Visit paradigm implementation for Result objects
     *
     * For non-void-value Result objects, the specified functor F will be
     * called as follows:
     * - F(GetValue()) if the object holds a value.
     * - F(GetError<E>()) if the object holds an error of type E.
     *
     * @exception UB if the object is empty (i.e. neither holds a value nor an error)
     */
    template <
        typename Visitor,
        typename = std::enable_if_t<
            std::is_invocable_v<Visitor, ValueType> &&
            (... && std::is_invocable_v<Visitor, ErrorTypes>)>>
    decltype(auto) Visit(Visitor&& visitor) {
        RESULT_ASSERT(!Base::IsEmpty(), "Visit() called on an empty object");
        return Base::template VisitArray<Visitor, void>::Call(
            std::forward<Visitor>(visitor), Base::Data(), Base::PhysicalIndex());
    }

    /**
     * @brief Visit paradigm implementation for Result objects
     * @see Visit, this is a const version of it
     */
    template <
        typename Visitor,
        typename = std::enable_if_t<
            std::is_invocable_v<Visitor, const ValueType> &&
            (... && std::is_invocable_v<Visitor, const ErrorTypes>)>>
    decltype(auto) Visit(Visitor&& visitor) const {
        RESULT_ASSERT(!Base::IsEmpty(), "Visit() called on an empty object");
        return Base::template VisitArray<Visitor, const void>::Call(
            std::forward<Visitor>(visitor), Base::Data(), Base::PhysicalIndex());
    }
};

template <typename... ErrorTypes>
struct VisitImpl<void, ErrorTypes...> : public DiscardValueImpl<void, ErrorTypes...> {
    using Base = DiscardValueImpl<void, ErrorTypes...>;

    /**
     * @brief Visit paradigm implementation for Result objects
     *
     * For void-value Result objects, the specified functor F will be called as follows:
     * - F() if the object is empty (holds a void value).
     * - F(GetError<E>()) if the object holds an error of type E.
     */
    template <
        typename Visitor,
        typename = std::enable_if_t<
            std::is_invocable_v<Visitor> && (... && std::is_invocable_v<Visitor, ErrorTypes>)>>
    decltype(auto) Visit(Visitor&& visitor) {
        if (!Base::HasAnyError()) {
            return std::forward<Visitor>(visitor)();
        } else {
            return Base::template VisitArray<Visitor, void>::Call(
                std::forward<Visitor>(visitor), Base::Data(), Base::PhysicalIndex());
        }
    }

    /**
     * @brief Visit paradigm implementation for Result objects
     * @see Visit, this is a const version of it
     */
    template <
        typename Visitor,
        typename = std::enable_if_t<
            std::is_invocable_v<Visitor> &&
            (... && std::is_invocable_v<Visitor, const ErrorTypes>)>>
    decltype(auto) Visit(Visitor&& visitor) const {
        if (!Base::HasAnyError()) {
            return std::forward<Visitor>(visitor)();
        } else {
            return Base::template VisitArray<Visitor, const void>::Call(
                std::forward<Visitor>(visitor), Base::Data(), Base::PhysicalIndex());
        }
    }
};

template <typename ValueType, typename... ErrorTypes>
using ResultImpl = VisitImpl<ValueType, ErrorTypes...>;

}  // namespace detail

/**
 * @brief A class representing either value or some error.
 *
 * The object of this class can be in 2 or 3 states, depending on ValueType template parameter:
 * - Empty;
 * - Has value: iff !std::is_same_v<void, ValueType>;
 * - Has error: iff sizeof...(ErrorTypes) > 0.
 *
 * The object can be created as empty or having value. In order to create an object holding an
 * error, one should use voe::MakeError.
 *
 * @exception None (the object does not produce any exceptions)
 */
template <typename ValueType, typename... ErrorTypes>
class [[nodiscard]] Result : public detail::ResultImpl<ValueType, ErrorTypes...> {
    using Base = detail::ResultImpl<ValueType, ErrorTypes...>;
    using SelfType = Result<ValueType, ErrorTypes...>;

 public:
    using value_type = ValueType;
    static_assert(detail::AllDecayed<ValueType, ErrorTypes...>, "All types must be decayed");
    static_assert(
        util::list::set::isaSet<util::list::list<ErrorTypes...>>,
        "Error types must not contain duplicates");

    /**
     * @brief Constructs an empty Result
     *
     * An empty Result neither holds error nor value.
     * Calls to such methods as GetError or GetValue will result in UB.
     * HasAnyError, HasError<*>, HasValue will return false;
     */
    Result() noexcept = default;

    /**
     * @brief Construct a Result holding a value
     *
     * The resulting object will store the value. HasValue will return true and
     * GetValue will be legal to use.
     *
     * @param from the value to be constructed from
     * @exception only ones thrown by ValueType's related copy/move constructor
     */
    template <
        typename FromType,
        typename = std::enable_if_t<std::is_same_v<ValueType, std::decay_t<FromType>>>>
    /* implicit */ Result(FromType&& from) noexcept(
        std::is_nothrow_copy_constructible_v<ValueType>) {
        Base::ValueConstruct(std::forward<FromType>(from));
    }

    Result(Result& voe) {
        Base::Construct(voe);
    }
    Result(const Result& voe) {
        Base::Construct(voe);
    }
    Result(Result&& voe) {
        Base::Construct(std::move(voe));
    }
    Result(const Result&& voe) {
        Base::Construct(std::move(voe));
    }

    /**
     * @brief Result conversion constructor
     *
     * The Result template instances are considered convertible iff:
     * - Their respective ValueType parameters are either same or void (one or both);
     * - The ErrorTypes... of from Result must be a subset of ErrorTypes... of to one.
     *
     * The rules are as follows:
     * - If from is empty, then the resulting object will be empty;
     * - If from holds a value (thus its ValueType is not void), and this type's ValueType is
     *   also not equal to void, than the value will be copied/moved to the resulting object;
     * - If from holds an error, than this error will be copied/moved to the resulting object.
     *
     * @exception (UB) from holds a value, and this type's ValueType is void
     * @exception Any exception thrown from copy or move constructor of respective type
     */
    template <
        typename FromVoe,
        typename = std::enable_if_t<detail::Convertible<
            detail::TransferTemplate<std::decay_t<FromVoe>, detail::list::list>,  //
            detail::list::list<ValueType, ErrorTypes...>>>>
    /* implicit */ Result(FromVoe&& from, void* = nullptr) {
        Base::ConvertConstruct(
            std::forward<FromVoe>(from), static_cast<std::decay_t<FromVoe>*>(nullptr));
    }

    SelfType& operator=(Result& arg) & {
        Base::Assign(arg);
        return *this;
    }
    SelfType& operator=(const Result& arg) & {
        Base::Assign(arg);
        return *this;
    }
    SelfType& operator=(Result&& arg) & {
        Base::Assign(std::move(arg));
        return *this;
    }
    SelfType& operator=(const Result&& arg) & {
        Base::Assign(std::move(arg));
        return *this;
    }

    /**
     * @brief Result conversion assignment operator
     *
     * See the conversion constructor operator in order to review the definition
     * of convertible Result template instances. The resulting object
     * will have the same properties as described in conversion constructor.
     *
     * The difference to conversion constructor is that if this object holds a value or an error,
     * it has to be either destroyed before construction of a new value or assigned to a new object,
     * depending on equality of types of objects that are held by lhs and rhs.
     *
     * @exception (UB) rhs holds a value, and this type's ValueType is void
     * @exception Any exception thrown from copy or move constructor of respective type
     */
    template <
        typename FromVoe,
        typename = std::enable_if_t<detail::Convertible<
            detail::TransferTemplate<std::decay_t<FromVoe>, detail::list::list>,  //
            detail::list::list<ValueType, ErrorTypes...>>>>
    SelfType& operator=(FromVoe&& rhs) & {
        Base::ConvertAssign(
            std::forward<FromVoe>(rhs), static_cast<std::decay_t<FromVoe>*>(nullptr));
        return *this;
    }

 protected:
    template <typename, typename...>
    friend class Result;
};

/**
 * @brief A shorter type template alias for void-returning functions
 */
template <typename... ErrorTypes>
using VoidOrError = Result<void, ErrorTypes...>;

/**
 * @brief A convenient and explicit way to create error objects
 * @param[in] ErrorType the type of an error
 * @param[in] error instance of #ErrorType&&
 * @return an instance of #Result<void, ErrorType> holding an error
 */
template <typename ErrorType, typename Decayed = std::decay_t<ErrorType>>
VoidOrError<Decayed> MakeError(ErrorType&& error) {
    VoidOrError<Decayed> result;
    result.SetError(std::forward<ErrorType>(error));
    return result;
}

/**
 * @brief A convenient and explicit way to create error objects
 * @param[in] ErrorType the type of an error
 * @param[in] Args types of #ErrorType constructor arguments
 * @param[in] args #ErrorType constructor arguments
 * @return an instance of #Result<void, ErrorType> holding an error
 */
template <typename ErrorType, typename... Args>
VoidOrError<ErrorType> MakeError(Args&&... args) {
    VoidOrError<ErrorType> result;
    result.template EmplaceError<ErrorType>(std::forward<Args>(args)...);
    return result;
}

}  // namespace result

#include "result/detail/unassert.h"
