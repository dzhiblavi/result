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

namespace detail_ {

static OpCollector* op_collector_{nullptr};

}  // namespace detail_

struct OpCollector {
    OpCollector() noexcept {
        assert(detail_::op_collector_ == nullptr);
        detail_::op_collector_ = this;
    }

    ~OpCollector() noexcept {
        assert(detail_::op_collector_ == this);
        detail_::op_collector_ = nullptr;
    }

    static void Push(Type type, int index) noexcept {
        if (detail_::op_collector_ == nullptr) {
            return;
        }
        detail_::op_collector_->ops.emplace_back(type, index);
    }

    template <typename... Args>
    bool Equal(Args&&... args) noexcept {
        std::array<Op, sizeof...(args)> compare_to{args...};

        bool equal = std::equal(ops.begin(), ops.end(), compare_to.begin(), compare_to.end());
        if (equal) {
            return true;
        }

        std::cerr << "Operation order comparison failed\n"
                  << "\tExpected: " << ToString(compare_to) << "\n"
                  << "\tFound:    " << ToString(ops) << std::endl;

        return false;
    }

    template <typename C>
    std::string ToString(const C& c) {
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
        OpCollector::Push(Type::Create, Index);
    }
    ~RememberLastOp() {
        OpCollector::Push(Type::Destroy, Index);
    }
    RememberLastOp(RememberLastOp&) {
        OpCollector::Push(Type::CONSTRUCT_COPY, Index);
    }
    RememberLastOp(const RememberLastOp&) {
        OpCollector::Push(Type::CONSTRUCT_COPY_CONST, Index);
    }
    RememberLastOp(RememberLastOp&&) noexcept {
        OpCollector::Push(Type::CONSTRUCT_MOVE, Index);
    }
    RememberLastOp(const RememberLastOp&&) noexcept {
        OpCollector::Push(Type::CONSTRUCT_MOVE_CONST, Index);
    }
    void operator=(RememberLastOp&) {
        OpCollector::Push(Type::ASSIGN_COPY, Index);
    }
    void operator=(const RememberLastOp&) {
        OpCollector::Push(Type::ASSIGN_COPY_CONST, Index);
    }
    void operator=(RememberLastOp&&) noexcept {
        OpCollector::Push(Type::ASSIGN_MOVE, Index);
    }
    void operator=(const RememberLastOp&&) noexcept {
        OpCollector::Push(Type::ASSIGN_MOVE_CONST, Index);
    }
    constexpr int GetIndex() const noexcept {
        return Index;
    }
};

template <int A, int B>
bool operator==(const RememberLastOp<A>&, const RememberLastOp<B>&) {
    return A == B;
}

}  // namespace test
