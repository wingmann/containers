#ifndef WINGMANN_CONTAINERS_DETAIL_TRAITS_ENABLE_ALLOCATOR_FROM_THIS_H
#define WINGMANN_CONTAINERS_DETAIL_TRAITS_ENABLE_ALLOCATOR_FROM_THIS_H

#include <utility>

namespace wingmann::containers::__detail::traits {

/// @brief A class to enable empty base optimization for allocators.
template<bool is_empty, typename Allocator>
class enable_allocator_from_this { };

/// @brief Store nothing when allocator is empty.
template<typename Allocator>
class enable_allocator_from_this<true, Allocator> {
public:
    constexpr enable_allocator_from_this() = default;

    constexpr explicit enable_allocator_from_this(const Allocator&)
    {
    }

    constexpr Allocator get_allocator() const noexcept
    {
        return Allocator();
    }

protected:
    constexpr void set_allocator(const Allocator&) const noexcept
    {
    }

    constexpr void set_allocator(Allocator&&) const noexcept
    {
    }

    constexpr void swap_allocator(Allocator&&) const noexcept
    {
    }
};

/// @brief Store allocator when allocator is not empty.
template<typename Allocator>
class enable_allocator_from_this<false, Allocator> {
    Allocator alloc_;

public:
    constexpr enable_allocator_from_this() : alloc_(Allocator())
    {
    }

    constexpr explicit enable_allocator_from_this(const Allocator &alloc) : alloc_(alloc)
    {
    }

    constexpr Allocator get_allocator() const noexcept
    {
        return alloc_;
    }

protected:
    constexpr void set_allocator(const Allocator &alloc) const noexcept
    {
        alloc_ = alloc;
    }

    constexpr void set_allocator(Allocator &&alloc) const noexcept
    {
        alloc_ = std::move(alloc);
    }

    constexpr void swap_allocator(Allocator &&alloc) const noexcept
    {
        std::swap(alloc_, alloc);
    }
};

} // namespace wingmann::containers::__detail::traits

#endif // WINGMANN_CONTAINERS_DETAIL_TRAITS_ENABLE_ALLOCATOR_FROM_THIS_H
