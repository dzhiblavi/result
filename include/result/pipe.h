#pragma once

#include "result/result.h"

#include <optional>

template <typename T, typename C, typename... Es>
auto operator|(result::Result<T, Es...> r, C c) {
    return std::move(c).pipe(std::move(r));
}

template <typename T, typename C>
auto operator|(std::optional<T> r, C c) {
    return std::move(c).pipe(std::move(r));
}
