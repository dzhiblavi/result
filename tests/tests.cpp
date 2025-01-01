#include "./remember_op.h"
#include "./voe_tools.h"

#include "result/result.h"

#include <gtest/gtest.h>
#include <type_traits>

namespace result {

template <typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename ValueType, typename VisitType, typename... ErrorTypes>
struct VisitChecker {
    static void check(Result<ValueType, ErrorTypes...>& voe) {
        voe.visit(overloaded{
            []() { FAIL() << "Unexpected visit of (void)"; },
            [](auto&&) { FAIL() << "Unexpected visit of other type"; },
            [](VisitType&) {},
        });
    }
};

struct DefaultConstructorTest {
    template <typename ValueType, typename... ErrorTypes>
    static void call(Result<ValueType, ErrorTypes...>*) {
        test::OpCollector collector;
        Result<ValueType, ErrorTypes...> voe;

        EXPECT_TRUE(collector.equal(test::Op(test::Create, ValueType::Idx)));
        EXPECT_TRUE(voe.hasValue());
        EXPECT_FALSE(voe.hasAnyError());
        EXPECT_FALSE(... || voe.template hasError<ErrorTypes>());
        VisitChecker<ValueType, ValueType, ErrorTypes...>::check(voe);
    }
};

struct ValueConstructorTest {
    template <typename ValueType, typename... ErrorTypes>
    static void call(Result<ValueType, ErrorTypes...>*) {
        test::OpCollector collector;
        Result<ValueType, ErrorTypes...> voe(ValueType{});

        EXPECT_TRUE(collector.equal(
            test::Op(test::Create, ValueType::Idx),
            test::Op(test::CONSTRUCT_MOVE, ValueType::Idx),
            test::Op(test::Destroy, ValueType::Idx)));

        EXPECT_TRUE(voe.hasValue());
        EXPECT_FALSE(voe.hasAnyError());
        EXPECT_FALSE(... || voe.template hasError<ErrorTypes>());
        EXPECT_EQ(ValueType{}, voe.value());
        VisitChecker<ValueType, ValueType, ErrorTypes...>::check(voe);
    }
};

struct MakeErrorTest {
    template <typename ValueType, typename... ErrorTypes>
    static void call(Result<ValueType, ErrorTypes...>* ptr) {
        (TestMakeError(ptr, static_cast<ErrorTypes*>(nullptr)), ...);
    }

    template <typename ValueType, typename... ErrorTypes, typename ErrorType>
    static void TestMakeError(Result<ValueType, ErrorTypes...>*, ErrorType*) {
        static constexpr size_t error_index = tl::Find<ErrorType, tl::List<ErrorTypes...>>;

        test::OpCollector collector;
        Result<ValueType, ErrorTypes...> voe{makeError<ErrorType>()};

        EXPECT_TRUE(collector.equal(
            test::Op(test::Create, ErrorType::Idx),
            test::Op(test::CONSTRUCT_MOVE, ErrorType::Idx),
            test::Op(test::Destroy, ErrorType::Idx)));

        EXPECT_TRUE(voe.hasAnyError());
        EXPECT_EQ(1 + error_index, voe.index());
        EXPECT_TRUE(voe.template hasError<ErrorType>());
        EXPECT_EQ(1, (... + int(voe.template hasError<ErrorTypes>())));
        EXPECT_EQ(ErrorType{}, voe.template error<ErrorType>());
        VisitChecker<ValueType, ErrorType, ErrorTypes...>::check(voe);
    }
};

struct CreateBase {
    template <typename... Types>
    struct First;
    template <typename T, typename... Ts>
    struct First<T, Ts...> {
        using type = T;
    };

    template <typename T>
    struct DefaultMarker {};

    template <typename T>
    struct ValueMarker {};

    template <typename T>
    struct ErrorMarker {};

    template <typename ValueType, typename... ErrorTypes, typename T>
    static Result<ValueType, ErrorTypes...> Create(
        Result<ValueType, ErrorTypes...>*, DefaultMarker<T>) {
        return T{};
    }

    template <typename ValueType, typename... ErrorTypes, typename T>
    static Result<ValueType, ErrorTypes...> Create(
        Result<ValueType, ErrorTypes...>*, ValueMarker<T>) {
        return T{};
    }

    template <typename ValueType, typename... ErrorTypes, typename T>
    static Result<ValueType, ErrorTypes...> Create(
        Result<ValueType, ErrorTypes...>*, ErrorMarker<T>) {
        return makeError(T{});
    }

    template <typename T, typename ValueType, typename... ErrorTypes>
    static Result<ValueType, ErrorTypes...> CreateDefault(Result<ValueType, ErrorTypes...>* ptr) {
        return Create(ptr, DefaultMarker<T>{});
    }

    template <typename ValueType, typename... ErrorTypes, typename T>
    static Result<ValueType, ErrorTypes...> CreateOther(
        Result<ValueType, ErrorTypes...>*, ValueMarker<T>) {
        return makeError(typename First<ErrorTypes...>::type{});
    }

    template <typename ValueType, typename... ErrorTypes, typename T>
    static Result<ValueType, ErrorTypes...> CreateOther(
        Result<ValueType, ErrorTypes...>*, ErrorMarker<T>) {
        return ValueType{};
    }

    template <typename ValueType, typename... ErrorTypes, typename T>
    static constexpr int OtherIndex(Result<ValueType, ErrorTypes...>*, ValueMarker<T>) {
        return First<ErrorTypes...>::type::Idx;
    }

    template <typename ValueType, typename... ErrorTypes, typename T>
    static constexpr int OtherIndex(Result<ValueType, ErrorTypes...>*, ErrorMarker<T>) {
        return ValueType::Idx;
    }
};

struct ConstructorsTest : CreateBase {
    template <
        typename ValueType1,
        typename... ErrorTypes1,
        typename ValueType2,
        typename... ErrorTypes2>
    static void call(
        tl::List<Result<ValueType1, ErrorTypes1...>, Result<ValueType2, ErrorTypes2...>>* p) {
        static constexpr bool both_non_void =
            !std::is_same_v<void, ValueType1> && !std::is_same_v<void, ValueType2>;

        // VoE(Empty): No operations expected
        TestConstructFrom(p, DefaultMarker<ValueType1>{});

        if constexpr (both_non_void) {
            // VoE(ValueTypeVoE{}): Copy/Move construction expected
            TestConstructFrom(p, ValueMarker<ValueType1>{});
        }

        // VoE(ErrorTypeVoE{}): Copy/Move construction expected
        (TestConstructFrom(p, ErrorMarker<ErrorTypes1>{}), ...);
    }

    template <typename T>
    static void check(test::OpCollector& collector, test::Type type, DefaultMarker<T>) {
        EXPECT_TRUE(collector.equal(test::Op(type, T::Idx)));
    }

    template <typename T>
    static void check(test::OpCollector& collector, test::Type type, ValueMarker<T>) {
        EXPECT_TRUE(collector.equal(test::Op(type, T::Idx)));
    }

    template <typename T>
    static void check(test::OpCollector& collector, test::Type type, ErrorMarker<T>) {
        EXPECT_TRUE(collector.equal(test::Op(type, T::Idx)));
    }

    template <
        typename ValueType1,
        typename... ErrorTypes1,
        typename ValueType2,
        typename... ErrorTypes2,
        typename ConstructAs>
    static void TestConstructFrom(
        tl::List<Result<ValueType1, ErrorTypes1...>, Result<ValueType2, ErrorTypes2...>>*,
        ConstructAs as) {
        using Result1 = Result<ValueType1, ErrorTypes1...>;
        using Result2 = Result<ValueType2, ErrorTypes2...>;
        {
            Result1 voe1{Create(static_cast<Result1*>(nullptr), as)};
            test::OpCollector collector;
            Result2 voe2{voe1};
            check(collector, test::CONSTRUCT_COPY, as);
        }
        {
            Result1 voe1{Create(static_cast<Result1*>(nullptr), as)};
            const Result1& voe1_cref = voe1;
            test::OpCollector collector;
            std::cout << "START " << __PRETTY_FUNCTION__ << std::endl;
            Result2 voe2 = voe1_cref;
            std::cout << "END" << std::endl;
            check(collector, test::CONSTRUCT_COPY_CONST, as);
        }
        {
            Result1 voe1{Create(static_cast<Result1*>(nullptr), as)};
            test::OpCollector collector;
            Result2 voe2{std::move(voe1)};
            check(collector, test::CONSTRUCT_MOVE, as);
        }
        {
            Result1 voe1{Create(static_cast<Result1*>(nullptr), as)};
            const Result1&& voe1_crref = std::move(voe1);
            test::OpCollector collector;
            Result2 voe2{std::move(voe1_crref)};
            check(collector, test::CONSTRUCT_MOVE_CONST, as);
        }
    }
};

struct AssignmentsTest : CreateBase {
    template <
        typename ValueType1,
        typename... ErrorTypes1,
        typename ValueType2,
        typename... ErrorTypes2>
    static void call(
        tl::List<Result<ValueType1, ErrorTypes1...>, Result<ValueType2, ErrorTypes2...>>*) {
        using Result1 = Result<ValueType1, ErrorTypes1...>;
        using Result2 = Result<ValueType2, ErrorTypes2...>;
        Result1* p1{nullptr};
        Result2* p2{nullptr};

        // X <- X
        // Assign to voe with some value from voe with same kind of value
        // Assignment(from) operator expected
        TestAssign(
            /* from = */ Create(p1, ValueMarker<ValueType1>{}),
            /* to =   */ Create(p2, ValueMarker<ValueType2>{}),
            ValueType1::Idx,
            test::ASSIGN_COPY,
            1);

        (TestAssign(
             /* from = */ Create(p1, ErrorMarker<ErrorTypes1>{}),
             /* to =   */ Create(p2, ErrorMarker<ErrorTypes1>{}),
             ErrorTypes1::Idx,
             test::ASSIGN_COPY,
             1),
         ...);

        if constexpr (sizeof...(ErrorTypes1) > 0) {
            // X <- Y
            // Assign to voe with some value from voe with other kind of value
            // Destructor(to) + Constructor(from) operator expected
            TestAssign(
                /* from = */ CreateOther(p1, ValueMarker<ValueType1>{}),
                /* to =   */ Create(p2, ValueMarker<ValueType2>{}),
                OtherIndex(p1, ValueMarker<ValueType1>{}),
                test::CONSTRUCT_COPY,
                1,
                test::Op(test::Destroy, ValueType2::Idx));

            (TestAssign(
                 /* from = */ CreateOther(p1, ErrorMarker<ErrorTypes1>{}),
                 /* to =   */ Create(p2, ErrorMarker<ErrorTypes1>{}),
                 OtherIndex(p1, ErrorMarker<ErrorTypes1>{}),
                 test::CONSTRUCT_COPY,
                 1,
                 test::Op(test::Destroy, ErrorTypes1::Idx)),
             ...);
        }
    }

    template <typename... Ops>
    static void check(test::OpCollector& collector, test::Type baseop, int index, Ops... ops) {
        if (baseop == test::None) {
            EXPECT_TRUE(collector.equal(ops...));
        } else {
            EXPECT_TRUE(collector.equal(ops..., test::Op(baseop, index)));
        }
    }

    template <
        typename ValueType1,
        typename... ErrorTypes1,
        typename ValueType2,
        typename... ErrorTypes2,
        typename... Ops>
    static void TestAssign(
        Result<ValueType1, ErrorTypes1...> v_from,
        Result<ValueType2, ErrorTypes2...> v_to,
        int index_from,
        test::Type baseop,
        int step,
        Ops... ops_prefix) {
        using Result1 = Result<ValueType1, ErrorTypes1...>;
        using Result2 = Result<ValueType2, ErrorTypes2...>;
        {
            Result2 voe{v_to};
            Result1 from{v_from};
            test::OpCollector collector;
            voe = from;
            check(collector, static_cast<test::Type>(baseop + 0 * step), index_from, ops_prefix...);
        }
        {
            Result2 voe{v_to};
            Result1 from{v_from};
            const Result1& from_cref = from;
            test::OpCollector collector;
            voe = from_cref;
            check(collector, static_cast<test::Type>(baseop + 1 * step), index_from, ops_prefix...);
        }
        {
            Result2 voe{v_to};
            Result1 from{v_from};
            test::OpCollector collector;
            voe = std::move(from);
            check(collector, static_cast<test::Type>(baseop + 2 * step), index_from, ops_prefix...);
        }
    }
};

TEST(DefaultConstructorTest, Correctness) {
    using Types = tl::Concat<
        test::Types<test::RememberLastOp<0>, test::RememberLastOp<0>, short, float>,
        test::Types<test::RememberLastOp<0>, test::RememberLastOp<0>, char, short>,
        test::Types<test::RememberLastOp<0>, test::RememberLastOp<0>>>;
    test::instantiateAndCall<DefaultConstructorTest>(Types{});
}

TEST(ValueConstructorTest, Correctness) {
    using Types = test::Types<
        test::RememberLastOp<0>,
        test::RememberLastOp<0>,
        int,
        char,
        std::string,
        const char*>;
    test::instantiateAndCall<ValueConstructorTest>(Types{});
}

TEST(MakeErrorTest, Correctness) {
    using Types = test::TypesWithErrors<
        test::RememberLastOp<0>,
        test::RememberLastOp<0>,
        test::RememberLastOp<1>,
        test::RememberLastOp<2>>;
    test::instantiateAndCall<MakeErrorTest>(Types{});
}

TEST(ConstructorsTest, CorrectTypeAndOperation) {
    using Types = test::AllConvertiblePairs<
        test::RememberLastOp<0>,
        test::RememberLastOp<0>,
        test::RememberLastOp<1>,
        test::RememberLastOp<2>>;
    test::instantiateAndCall<ConstructorsTest>(Types{});
}

TEST(AssignmentsTest, CorrectTypeAndOperation) {
    using Types = test::AllConvertiblePairs<
        test::RememberLastOp<0>,
        test::RememberLastOp<0>,
        test::RememberLastOp<1>,
        test::RememberLastOp<2>>;
    test::instantiateAndCall<AssignmentsTest>(Types{});
}

}  // namespace result
