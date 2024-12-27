#pragma once

#include <cassert>
#include <optional>
#include <utility>

namespace voe::detail {

template <typename T>
class ReturnedObjectHolder {
 public:
    explicit ReturnedObjectHolder(ReturnedObjectHolder** owner) : owner_(owner) {
        *owner_ = this;
    }

    // Pinned
    ReturnedObjectHolder(ReturnedObjectHolder&&) = delete;
    ReturnedObjectHolder(ReturnedObjectHolder const&) = delete;
    ReturnedObjectHolder& operator=(ReturnedObjectHolder const&) = delete;
    ReturnedObjectHolder& operator=(ReturnedObjectHolder&&) = delete;

    void* ptr() {
        return &object_;
    }

    void assign(T object) {
        assert(!object_.has_value());
        object_.emplace(std::move(object));
    }

    /* implicit */ operator T() && {
        assert(object_.has_value());
        return std::move(*object_);
    }

 private:
    std::optional<T> object_ = std::nullopt;
    ReturnedObjectHolder** owner_ = nullptr;
};

}  // namespace voe::detail
