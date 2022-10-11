#ifndef WINGMANN_CONTAINERS_DETAIL_ITERATORS_POINTER_WRAPPER_H
#define WINGMANN_CONTAINERS_DETAIL_ITERATORS_POINTER_WRAPPER_H

#include <algorithm>
#include <iterator>
#include <type_traits>

#include <concepts>

namespace wingmann::containers::__detail::iterators {

/// @brief Wraps a pointer as an iterator.
/// @tparam Pointer
///
template<typename Pointer>
requires std::is_pointer_v<Pointer>
class pointer_wrapper {
public:
    using iterator_type     = Pointer;
    using iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;
    using value_type        = typename std::iterator_traits<iterator_type>::value_type;
    using difference_type   = typename std::iterator_traits<iterator_type>::difference_type;
    using pointer           = typename std::iterator_traits<iterator_type>::pointer;
    using reference         = typename std::iterator_traits<iterator_type>::reference;

private:
    // Base pointer.
    iterator_type base_;

public:
    /// @brief Construct empty pointer wrapper.
    constexpr pointer_wrapper() noexcept : base_(nullptr)
    {
    }

    /// @brief Construct pointer wrapper from pointer.
    /// @param ptr Pointer.
    ///
    constexpr explicit pointer_wrapper(iterator_type ptr) noexcept : base_{ptr}
    {
    }

    /// \brief Construct pointer wrapper from pointer wrapper u, which
    /// might be another type
    template<std::convertible_to<iterator_type> UP>
    constexpr pointer_wrapper(const pointer_wrapper<UP>& pw) noexcept : base_{pw.base()}
    {
    }

public:
    /// @brief Dereference iterator.
    constexpr reference operator*() const noexcept
    {
        return *base_;
    }

    /// \brief Dereference iterator and get member
    constexpr pointer operator->() const noexcept
    {
        return static_cast<pointer>(std::addressof(*base_));
    }

    /// @brief Dereference iterator n positions ahead.
    constexpr reference operator[](difference_type position) const noexcept
    {
        return base_[position];
    }

    /// @brief Get base pointer.
    constexpr iterator_type base() const noexcept
    {
        return base_;
    }

public:
    /// @brief Advance iterator.
    constexpr pointer_wrapper& operator++() noexcept
    {
        ++base_;
        return *this;
    }

    /// @brief Advance iterator.
    constexpr pointer_wrapper operator++(int) noexcept
    {
        pointer_wrapper tmp(*this);
        ++(*this);
        return tmp;
    }

    /// @brief Rewind iterator.
    constexpr pointer_wrapper& operator--() noexcept
    {
        --base_;
        return *this;
    }

    /// @brief Rewind iterator.
    constexpr pointer_wrapper operator--(int) noexcept
    {
        pointer_wrapper tmp(*this);
        --(*this);
        return tmp;
    }

    /// @brief Return copy of iterator advanced by n positions.
    constexpr pointer_wrapper operator+(difference_type n) const noexcept
    {
        pointer_wrapper w(*this);
        w += n;
        return w;
    }

    /// @brief Advance iterator by n positions.
    constexpr pointer_wrapper& operator+=(difference_type n) noexcept
    {
        base_ += n;
        return *this;
    }

    /// @brief Return copy of iterator n positions behind.
    constexpr pointer_wrapper operator-(difference_type n) const noexcept
    {
        return *this + (-n);
    }

    /// @brief Rewind iterator by n positions.
    constexpr pointer_wrapper& operator-=(difference_type n) noexcept
    {
        *this += -n;
        return *this;
    }

public:
    /// Make any other pointer wrapper a friend.
    template<typename UP>
    friend class pointer_wrapper;

public:
    /// @brief Get distance between iterators.
    template<typename Iter1, typename Iter2>
    constexpr friend auto operator-(
            const pointer_wrapper<Iter1>& x,
            const pointer_wrapper<Iter2>& y) noexcept -> decltype(x.base() - y.base());

    /// @brief Sum iterators.
    template<typename Iter1>
    constexpr friend pointer_wrapper<Iter1> operator+(
            typename pointer_wrapper<Iter1>::difference_type,
            pointer_wrapper<Iter1>) noexcept;
};

template<typename Iter1, typename Iter2>
constexpr bool operator==(const pointer_wrapper<Iter1>& x, const pointer_wrapper<Iter2>& y) noexcept
{
    return x.base() == y.base();
}

template<typename Iter1, typename Iter2>
constexpr bool operator!=(const pointer_wrapper<Iter1>& x, const pointer_wrapper<Iter2>& y) noexcept
{
    return !(x == y);
}

template<typename Iter1, typename Iter2>
constexpr bool operator<(const pointer_wrapper<Iter1>& x, const pointer_wrapper<Iter2>& y) noexcept
{
    return x.base() < y.base();
}

template<typename Iter1, typename Iter2>
constexpr bool operator>(const pointer_wrapper<Iter1>& x, const pointer_wrapper<Iter2>& y) noexcept
{
    return y < x;
}

template<typename Iter1, typename Iter2>
constexpr bool operator>=(const pointer_wrapper<Iter1>& x, const pointer_wrapper<Iter2>& y) noexcept
{
    return !(x < y);
}

template<typename Iter1, typename Iter2>
constexpr bool operator<=(const pointer_wrapper<Iter1>& x, const pointer_wrapper<Iter2>& y) noexcept
{
    return !(y < x);
}

template<typename Iter>
constexpr bool operator==(const pointer_wrapper<Iter>& x, const pointer_wrapper<Iter>& y) noexcept
{
    return x.base() == y.base();
}

template<typename Iter>
constexpr bool operator!=(const pointer_wrapper<Iter>& x, const pointer_wrapper<Iter>& y) noexcept
{
    return !(x == y);
}

template<typename Iter>
constexpr bool operator>(const pointer_wrapper<Iter>& x, const pointer_wrapper<Iter>& y) noexcept
{
    return y < x;
}

template<typename Iter>
constexpr bool operator>=(const pointer_wrapper<Iter>& x, const pointer_wrapper<Iter>& y) noexcept
{
    return !(x < y);
}

template<typename Iter>
constexpr bool operator<=(const pointer_wrapper<Iter>& x, const pointer_wrapper<Iter>& y) noexcept
{
    return !(y < x);
}

template<typename Iter, typename BaseIter>
constexpr bool operator==(const pointer_wrapper<Iter>& x, const BaseIter& y) noexcept
{
    return x.base() == y;
}

template<typename Iter, typename BaseIter>
constexpr bool
operator!=(const pointer_wrapper<Iter>& x, const BaseIter& y) noexcept
{
    return !(x == y);
}

template<typename Iter, typename BaseIter>
constexpr bool
operator>(const pointer_wrapper<Iter>& x, const BaseIter& y) noexcept
{
    return y < x;
}

template<typename Iter, typename BaseIter>
constexpr bool operator>=(const pointer_wrapper<Iter>& x, const BaseIter& y) noexcept
{
    return !(x < y);
}

template<typename Iter, typename BaseIter>
constexpr bool operator<=(const pointer_wrapper<Iter>& x, const BaseIter& y) noexcept
{
    return !(y < x);
}

template<typename Iter, typename BaseIter>
constexpr bool operator==(const BaseIter& x, const pointer_wrapper<Iter>& y) noexcept
{
    return x.base() == y.base();
}

template<typename Iter, typename BaseIter>
constexpr bool operator!=(const BaseIter& x, const pointer_wrapper<Iter>& y) noexcept
{
    return !(x == y);
}

template<typename Iter, typename BaseIter>
constexpr bool operator>(const BaseIter& x, const pointer_wrapper<Iter>& y) noexcept
{
    return y < x;
}

template<typename Iter, typename BaseIter>
constexpr bool operator>=(const BaseIter& x, const pointer_wrapper<Iter>& y) noexcept
{
    return !(x < y);
}

template<typename Iter, typename BaseIter>
constexpr bool operator<=(const BaseIter& x, const pointer_wrapper<Iter>& y) noexcept
{
    return !(y < x);
}

template<typename Iter1, typename Iter2>
constexpr auto operator-(const pointer_wrapper<Iter1>& x, const pointer_wrapper<Iter2>& y) noexcept
    -> decltype(x.base() - y.base())
{
    return x.base() - y.base();
}

template<typename Iter>
constexpr pointer_wrapper<Iter> operator+(
    typename pointer_wrapper<Iter>::difference_type n,
    pointer_wrapper<Iter> x) noexcept
{
    x += n;
    return x;
}

} // namespace wingmann::containers::__detail::iterators

#endif // WINGMANN_CONTAINERS_DETAIL_ITERATORS_POINTER_WRAPPER_H
