#pragma once

#include "voe/detail/templates/impl.h"

#include <type_traits>

namespace util::tpl::unit {

static_assert(std::is_same_v<list::list<>, applyToTemplate<list::list, list::list<>>>);
static_assert(std::is_same_v<list::list<int>, applyToTemplate<list::list, list::list<int>>>);
static_assert(
    std::is_same_v<list::list<int, float>, applyToTemplate<list::list, list::list<int, float>>>);

}  // namespace util::tpl::unit
