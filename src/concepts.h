#pragma once

#include <type_traits>
#include <ranges>

template <class Range>
concept IntegerRange = std::ranges::common_range<Range> and std::is_same_v<std::ranges::range_value_t<Range>, std::size_t>;

template <class T, class... Ti>
concept is_any_of = (std::is_same_v<T, Ti> or ...);

template <class T>
concept dependent_false_v = false;
