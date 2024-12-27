#pragma once

#include "voe/detail/type_list/impl.h"

namespace util::list::unit {

static_assert(std::is_same_v<concat<>, list<>>);
static_assert(std::is_same_v<concat<list<>, list<>>, list<>>);
static_assert(std::is_same_v<concat<list<>, list<int>, list<>>, list<int>>);
static_assert(std::is_same_v<concat<list<int>, list<>, list<float>>, list<int, float>>);

static_assert(std::is_same_v<list<>, flatten<list<list<>, list<>, list<>>>>);
static_assert(std::is_same_v<list<int>, flatten<list<list<>, list<int>, list<>>>>);
static_assert(std::is_same_v<
              list<int, float, int, int>,
              flatten<list<list<int>, list<float>, list<int, int>>>>);

static_assert(std::is_same_v<list<>, set::product<list<>, list<>>>);
static_assert(std::is_same_v<list<>, set::product<list<int>, list<>>>);
static_assert(std::is_same_v<list<list<int, int>>, set::product<list<int>, list<int>>>);
static_assert(
    std::is_same_v<list<list<int, char, float>>, set::product<list<int>, list<char>, list<float>>>);
static_assert(std::is_same_v<
              list<list<int, char>, list<int, short>, list<float, char>, list<float, short>>,
              set::product<list<int, float>, list<char, short>>>);

static_assert(std::is_same_v<list<>, set::unique<list<>>>);
static_assert(std::is_same_v<
              list<char, float, int>,
              set::unique<list<int, int, float, char, int, char, float, int>>>);

static_assert(std::is_same_v<list<>, set::intersect<list<>, list<>>>);
static_assert(std::is_same_v<list<>, set::intersect<list<int>, list<float>>>);
static_assert(std::is_same_v<list<int>, set::intersect<list<int, char>, list<float, int>>>);
static_assert(
    std::is_same_v<list<int, float>, set::intersect<list<int, char, float>, list<float, int>>>);

static_assert(std::is_same_v<list<>, set::unite<list<>, list<>>>);
static_assert(std::is_same_v<list<int>, set::unite<list<>, list<int>>>);
static_assert(std::is_same_v<list<int>, set::unite<list<int>, list<>>>);
static_assert(std::is_same_v<list<int>, set::unite<list<int>, list<int>>>);
static_assert(std::is_same_v<list<float, int>, set::unite<list<int, float>, list<float, int>>>);

}  // namespace util::list::unit
