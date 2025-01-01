#pragma once

#include "result/result.h"

#include <optional>

namespace result {

namespace pipe {

template <typename E>
struct Lift {
    E error;

    template <typename T, typename Self>
    Result<T, E> pipe(this Self&& self, std::optional<T> opt) {
        if (opt.has_value()) {
            return std::move(*opt);
        }

        return makeError<E>(std::forward<Self>(self).error);
    }
};

}  // namespace pipe

// Maybe<T> -> Result<T, E>
template <typename E>
auto lift(E err) {
    return pipe::Lift(std::move(err));
}

template <typename E, typename... Args>
auto lift(Args&&... args) {
    return pipe::Lift(E{std::forward<Args>(args)...});
}

}  // namespace result
