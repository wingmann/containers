#ifndef WINGMANN_CONTAINERS_DETAIL_TRAITS_IS_RANGE_H
#define WINGMANN_CONTAINERS_DETAIL_TRAITS_IS_RANGE_H

#include <concepts>

namespace wingmann::containers::__detail::traits {

/// @brief Check if type is a range (has begin() and end() functions).
template<typename T, typename = void>
struct is_range : std::false_type { };

template <typename T>
struct is_range<
    T,
    std::void_t<decltype(
        std::declval<T>().begin()),
        decltype(std::declval<T>().end()),
        typename T::value_type>> : std::true_type { };

/// @brief True if type is a range (has begin() and end() functions).
template <typename T>
constexpr bool is_range_v = is_range<std::decay_t<T>>::value;

} // namespace wingmann::containers::__detail::traits

#endif // WINGMANN_CONTAINERS_DETAIL_TRAITS_IS_RANGE_H
