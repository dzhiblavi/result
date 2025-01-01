#pragma once

#include <array>
#include <cassert>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

namespace test {

enum Type {
    None,
    Create,
    Destroy,
    CONSTRUCT_COPY,
    CONSTRUCT_COPY_CONST,
    CONSTRUCT_MOVE,
    CONSTRUCT_MOVE_CONST,
    ASSIGN_COPY,
    ASSIGN_COPY_CONST,
    ASSIGN_MOVE,
    ASSIGN_MOVE_CONST,
};

using Op = std::pair<Type, int>;

struct OpCollector;

namespace detail {

static OpCollector* op_collector_{nullptr};  // NOLINT

}  // namespace detail

struct OpCollector {
    OpCollector() noexcept {
        assert(detail::op_collector_ == nullptr);
        detail::op_collector_ = this;
    }

    ~OpCollector() noexcept {
        assert(detail::op_collector_ == this);
        detail::op_collector_ = nullptr;
    }

    static void push(Type type, int index) noexcept {
        if (detail::op_collector_ == nullptr) {
            return;
        }
        detail::op_collector_->ops.emplace_back(type, index);
    }

    template <typename... Args>
    bool equal(const Args&... args) noexcept {
        std::array<Op, sizeof...(args)> compare_to{args...};

        bool equal = std::equal(ops.begin(), ops.end(), compare_to.begin(), compare_to.end());
        if (equal) {
            return true;
        }

        std::cerr << "Operation order comparison failed\n"
                  << "\tExpected: " << toString(compare_to) << "\n"
                  << "\tFound:    " << toString(ops) << std::endl;

        return false;
    }

    template <typename C>
    std::string toString(const C& c) {
        std::stringstream ss;
        ss << "[";
        for (const auto& [type, idx] : c) {
            ss << "(" << type << "," << idx << ") ";
        }
        ss << "\b]";
        return ss.str();
    }

    std::vector<Op> ops;
};

template <int Index>
struct RememberLastOp {
    static constexpr int Idx = Index;

    RememberLastOp() {
        OpCollector::push(Type::Create, Index);
    }

    ~RememberLastOp() {
        OpCollector::push(Type::Destroy, Index);
    }

    RememberLastOp(RememberLastOp&) {
        OpCollector::push(Type::CONSTRUCT_COPY, Index);
    }

    RememberLastOp(const RememberLastOp&) {
        OpCollector::push(Type::CONSTRUCT_COPY_CONST, Index);
    }

    RememberLastOp(RememberLastOp&&) noexcept {
        OpCollector::push(Type::CONSTRUCT_MOVE, Index);
    }

    RememberLastOp(const RememberLastOp&&) noexcept {
        OpCollector::push(Type::CONSTRUCT_MOVE_CONST, Index);
    }

    RememberLastOp& operator=(RememberLastOp&) {  // NOLINT
        OpCollector::push(Type::ASSIGN_COPY, Index);
        return *this;
    }

    RememberLastOp& operator=(const RememberLastOp&) {
        OpCollector::push(Type::ASSIGN_COPY_CONST, Index);
        return *this;
    }

    RememberLastOp& operator=(RememberLastOp&&) noexcept {
        OpCollector::push(Type::ASSIGN_MOVE, Index);
        return *this;
    }

    constexpr int getIndex() const noexcept {
        return Index;
    }
};

template <int A, int B>
bool operator==(const RememberLastOp<A>&, const RememberLastOp<B>&) {
    return A == B;
}

}  // namespace test
