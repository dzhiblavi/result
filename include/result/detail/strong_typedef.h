#pragma once

#include <type_traits>
#include <utility>

namespace result::detail {

template <class Self, typename T>
class StrongTypedef {
 public:
    template <typename... Args>
    StrongTypedef(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : value_(std::forward<Args>(args)...) {}

    template <typename S>
    decltype(auto) get(this S&& self) noexcept {  // NOLINT
        return std::forward_like<S>(self.value_);
    }

 private:
    T value_;
};

}  // namespace result::detail
