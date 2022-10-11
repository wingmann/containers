#ifndef WINGMANN_CONTAINERS_DETAIL_TRAITS_IS_RELOCATABLE_H
#define WINGMANN_CONTAINERS_DETAIL_TRAITS_IS_RELOCATABLE_H

#include <ranges>

namespace wingmann::containers::__detail::traits {

template <class T>
struct is_relocatable : std::conjunction<
    std::is_trivially_copy_constructible<T>,
    std::is_trivially_copy_assignable<T>,
    std::is_trivially_destructible<T>> { };

template <class T>
constexpr bool is_relocatable_v = is_relocatable<T>::value;

} // namespace wingmann::containers::__detail::traits

#endif // WINGMANN_CONTAINERS_DETAIL_TRAITS_IS_RELOCATABLE_H
