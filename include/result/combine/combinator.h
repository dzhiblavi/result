#pragma once

#include "result/result.h"

namespace result {

template <typename C>
concept Combinator = requires(C c, Result<int, int> r) {
    { c.pipe(std::move(r)) };
};

}  // namespace result
