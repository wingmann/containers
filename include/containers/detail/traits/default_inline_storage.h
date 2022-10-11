#ifndef WINGMANN_CONTAINERS_DETAIL_TRAITS_DEFAULT_INLINE_STORAGE_H
#define WINGMANN_CONTAINERS_DETAIL_TRAITS_DEFAULT_INLINE_STORAGE_H

#include <cstddef>
#include <cmath>

namespace wingmann::containers::__detail::traits {

namespace {

constexpr std::size_t expected_min_reasonable_inline_vector = 5;

template <class T>
constexpr std::size_t expected_inline_values_per_heap_pointers =
        (sizeof(T*) + sizeof(size_t)) / sizeof(T);

}

template <typename T>
struct default_inline_storage : std::integral_constant<
    size_t,
    std::max(
        expected_min_reasonable_inline_vector,
        expected_inline_values_per_heap_pointers<T>)> { };

template <class T>
constexpr std::size_t default_inline_storage_v = default_inline_storage<T>::value;

} // namespace wingmann::containers::__detail::traits

#endif // WINGMANN_CONTAINERS_DETAIL_TRAITS_DEFAULT_INLINE_STORAGE_H
