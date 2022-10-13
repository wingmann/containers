/// @file   vector.h
/// @author Alexander Shavrov (alexander.shavrov@outlook.com)
/// @brief  Vector data structure implementation.
/// @date   2022-10-13
///
/// @copyright Copyright (c) 2022
///
/// This file is distributed under the MIT License.
/// See LICENSE file for details.
///

#ifndef WINGMANN_CONTAINERS_VECTOR
#define WINGMANN_CONTAINERS_VECTOR

#include <iterators/random_access_iterator.h>

#include <algorithm>
#include <compare>
#include <concepts>
#include <exception>
#include <initializer_list>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace wingmann::containers {

template<typename Type, typename Allocator = std::allocator<Type>>
class vector {
public:
    // Aliases
    using value_type             = Type;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using pointer                = Type*;
    using const_pointer          = const pointer;
    using allocator_type         = Allocator;
    using const_alloc_reference  = const allocator_type&;
    using size_type              = std::size_t;
    using init_list_type         = std::initializer_list<Type>;
    using iterator               = iterators::random_access_iterator<Type>;
    using const_iterator         = iterators::random_access_iterator<const Type>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type        = std::ptrdiff_t;

private:
    pointer data_;
    size_type size_{};
    size_type capacity_{};
    allocator_type allocator_;

public:
    // Constructors --------------------------------------------------------------------------------

    /// @brief Default constructor.
    constexpr vector() noexcept : data_{}
    {
    }

    /// @brief Copy constructor.
    constexpr vector(const vector& other)
    {
        allocator_ = std::allocator_traits<allocator_type>::select_on_container_copy_construction(
            other.get_allocator());

        copy(other);
    }

    /// @brief Move constructor.
    constexpr vector(vector&& other) noexcept
        : data_{other.data_},
          size_{other.size_},
          capacity_{other.capacity_},
          allocator_{std::move(other.allocator_)}
    {
        reset(other);
    }

    ~vector() noexcept
    {
        deallocate_and_destruct(capacity_, size_);
    }

    constexpr explicit vector(const_alloc_reference allocator) noexcept
        : allocator_{allocator }, data_{}
    {
    }

    constexpr explicit vector(size_type length, const_alloc_reference allocator = Allocator())
        : allocator_{allocator }
    {
        allocate_and_copy_construct(length, length);
    }

    constexpr explicit vector(
        size_type length,
        const_reference value,
        const_alloc_reference allocator = Allocator()) : allocator_{allocator }
    {
        allocate_and_copy_construct(length, length, value);
    }

    constexpr explicit vector(init_list_type values, const_alloc_reference allocator = Allocator())
        : allocator_{allocator }
    {
        allocate(values.size());
        construct_init_list(values);
    }

    template<typename InputIter>
    constexpr vector(InputIter first, InputIter last)
    {
        size_type size = std::distance(last, first);
        allocate(size);

        for (size_type index = 0; index < size; ++index) {
            std::allocator_traits<Allocator>::construct(
                allocator_,
                data_ + index,
                *(first + index));

            ++size_;
        }
    }

    constexpr vector(const vector& other, const_alloc_reference allocator) : allocator_{allocator }
    {
        copy(other);
    }

    /// @brief Move constructor.
    constexpr vector(vector&& other, const_alloc_reference allocator) noexcept
        : size_{other.size_ }, capacity_{other.capacity_ }, allocator_{std::move(allocator) }
    {
        if (allocator != other.get_allocator())
            uninitialized_alloc_move(std::move(other));
        else
            data_ = other.data_;

        reset(other);
    }

    // Assignment operators ------------------------------------------------------------------------

    /// @brief Copy assignment operator.
    constexpr vector& operator=(const vector& other)
    {
        if (this == &other)
            return *this;

        destruct(size());

        if (other.data_)
        {
            if constexpr (std::allocator_traits<allocator_type>
            ::propagate_on_container_copy_assignment::value)
            {
                allocator_ = other.get_allocator();
            }

            if (other.size() > capacity())
                reallocate(capacity(), other.size());

            uninitialized_alloc_copy(other);
        }
        else {
            data_ = nullptr;
            size_ = 0;
        }
        return *this;
    }

    /// @brief Move assignment operator
    constexpr vector& operator=(vector&& other) noexcept
    {
        if (this == &other) return *this;

        if constexpr (std::allocator_traits<allocator_type>
            ::propagate_on_container_move_assignment::value)
        {
            deallocate_and_destruct(capacity(), size());
            allocator_ = other.get_allocator();
            data_ = other.data_;
            reset(other);
        }
        else if (allocator_ == other.allocator_) {
            deallocate_and_destruct(capacity(), size());
            data_ = other.data_;
            reset(other);
        }
        else {
            destruct(size());
            reallocate(capacity(), other.capacity());
            uninitialized_alloc_move(std::move(other));
        }
        size_ = other.size_;
        capacity_ = other.capacity_;

        return *this;
    }

    /// @brief Assignment operator.
    constexpr vector& operator=(init_list_type values)
    {
        destruct(size_);

        if (values.size() > capacity())
            reallocate(capacity(), values.size());

        construct_init_list(values);
        return *this;
    }

    // Comparison operators ------------------------------------------------------------------------

    constexpr friend bool operator==(const vector& first, const vector& second)
    {
        return (first.size_ == second.size_ && std::equal(
                first.data_,
                first.data_ + first.size(),
                second.data_,
                second.data_ + second.size()));
    }

    constexpr bool operator!=(const vector& other) const
    {
        return *this != other;
    }

    constexpr bool operator<(const vector& other) const
    {
        return std::lexicographical_compare(data_, data_ + size(), other.begin(), other.end());
    }

    constexpr bool operator>(const vector& other) const
    {
        return *this >= other;
    }

    constexpr bool operator<=(const vector& other) const
    {
        return other >= *this;
    }

    constexpr bool operator>=(const vector& other) const
    {
        return *this >= other;
    }

    // Methods -------------------------------------------------------------------------------------

    constexpr void assign(size_type size, const_reference value)
    {
        destruct(size_);

        if (size > capacity())
            reallocate(capacity(), size);

        construct(size, value);
    }

    constexpr void assign(init_list_type values)
    {
        destruct(size_);

        if (values.size() > capacity())
            reallocate(capacity(), values.size());

        construct_init_list(values);
    }

    template<typename input_iter>
    constexpr void assign(input_iter first, input_iter last)
    {
        size_type size = std::distance(last, first);
        destruct(size);

        if (size > capacity())
            reallocate(capacity(), size);

        for (size_type index = 0; index < size; ++index) {
            std::allocator_traits<Allocator>::construct(
                allocator_,
                data_ + index,
                *(first + index));

            ++size_;
        }
    }

    constexpr allocator_type get_allocator() const noexcept
    {
        return allocator_;
    }

    // Access methods ------------------------------------------------------------------------------

    constexpr reference at(size_type index)
    {
        return index < size() ? data_[index] : throw std::out_of_range("Index out of range");
    }

    constexpr const_reference at(size_type index) const
    {
        return index < size()
            ? data_[index]
            : throw std::out_of_range("Index out of range");
    }

    constexpr reference operator[](size_type index)
    {
        static_assert(index < size() && "Index out of range");
        return data_[index];
    }

    constexpr const_reference operator[](size_type index) const
    {
        static_assert(index < size() && "Index out of range"); return data_[index];
    }

    constexpr pointer data() noexcept
    {
        return (size() != 0) ? data_ : nullptr;
    }

    constexpr const_pointer data() const noexcept
    {
        return (size() != 0) ? data_ : nullptr;
    }

    constexpr reference back()
    {
        return *(end() - 1);
    }

    constexpr const_reference back() const
    {
        return *(end() - 1);
    }

    constexpr reference front()
    {
        return *(begin());
    }

    constexpr const_reference front() const
    {
        return *(begin());
    }

    // Iterators -----------------------------------------------------------------------------------

    constexpr iterator begin() noexcept
    {
        return data_;
    }

    constexpr const_iterator begin() const noexcept
    {
        return data_;
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return data_;
    }

    constexpr reverse_iterator rbegin() noexcept
    {
        return reverse_iterator(data_ + size());
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(data_ + size());
    }

    constexpr iterator end() noexcept
    {
        return data_ + size();
    }

    constexpr const_iterator end() const noexcept
    {
        return data_ + size();
    }

    constexpr const_iterator cend() const noexcept
    {
        return data_ + size();
    }

    constexpr reverse_iterator rend() noexcept
    {
        return reverse_iterator(data_);
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
        return reverse_iterator(data_);
    }

    // Capacity related ----------------------------------------------------------------------------

    [[nodiscard]]
    constexpr size_type size() const noexcept
    {
        return size_;
    }

    [[nodiscard]]
    constexpr size_type max_size() const noexcept
    {
        return std::numeric_limits<difference_type>::max();
    }

    [[nodiscard]]
    constexpr size_type capacity() const noexcept
    {
        return capacity_;
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept
    {
        return size_ == 0;
    }

    [[nodiscard]]
    constexpr bool is_null() const noexcept
    {
        return data_ == nullptr;
    }

    constexpr void reserve(size_type capacity)
    {
        if (capacity > max_size())
            throw std::length_error("Capacity allocated exceeds max_size()");
        else if (capacity > capacity_)
            reallocate_strong_guarantee(capacity);
    }

    constexpr void shrink_to_fit()
    {
        if (capacity_ != size_)
            reallocate_strong_guarantee(size_);
    }

    // Modifier functions
    constexpr void clear() noexcept
    {
        destruct(size_);
    }

    constexpr iterator insert(const iterator pos, const_reference value)
    {
        return emplace(pos, value);
    }

    constexpr iterator insert(const iterator pos, value_type&& value)
    {
        return emplace(pos, std::move(value));
    }

    constexpr iterator insert(const iterator pos, init_list_type values)
    {
        size_type pos_index_position = std::distance(pos, begin());

        if (size() + values.size() < capacity()) {
            shift_and_construct_init(pos_index_position, values);
        }
        else {
            do {
                if (capacity_ == 0)
                    capacity_ = 1;

                capacity_ *= realloc_factor();
            } while (capacity_ < values.size() + size_);

            reallocate_strong_guarantee(capacity_);
            shift_and_construct_init(pos_index_position, values);
        }
        return values.size() == 0 ? pos : iterator(data_ + pos_index_position);
    }

    constexpr iterator insert(const iterator pos, size_type count, const_reference value)
    {
        size_type pos_index_position = std::distance(pos, begin());

        if (size() + count < capacity()) {
            if (pos == end())
                insert_end_strong_guarantee(value);
            else
                shift_and_construct(pos_index_position, value, count);
        }
        else {
            do {
                if (capacity_ == 0) capacity_ = 1;
                capacity_ *= realloc_factor();
            } while (capacity_ < size_);

            reallocate_strong_guarantee(capacity_);
            shift_and_construct(pos_index_position, value, count);
        }
        return count == 0 ? pos : iterator(data_ + pos_index_position);
    }

    constexpr iterator erase(const iterator pos) {
        static_assert(pos <= end() && "Vector subscript out of range");
        size_type pos_index_position = std::distance(pos, begin());
        std::allocator_traits<allocator_type>::destroy(allocator_, data_ + pos_index_position);

        if constexpr (std::is_nothrow_move_constructible<Type>::value)
            std::move(data_ + pos_index_position + 1, data_ + size(), data_ + pos_index_position);
        else
            std::copy(data_ + pos_index_position + 1, data_ + size(), data_ + pos_index_position);

        --size_;
        return (end() == pos) ? end() : iterator(data_ + pos_index_position);
    }

    constexpr iterator erase(const iterator first, const iterator last)
    {
        bool last_equals_end = (last == end());

        static_assert(first <= end() && "Vector's first argument out of range");
        static_assert(last <= end() && "Vector's second argument out of range");
        static_assert(first <= last && "Vector's first argument smaller than second argument");

        size_type first_position = std::distance(first, begin());
        size_type last_position = std::distance(last, begin());
        size_type difference{ last_position - first_position };

        for (size_type index{ first_position }; index < last_position; ++index)
            std::allocator_traits<allocator_type>::destroy(allocator_, data_ + index);

        if constexpr (std::is_nothrow_move_constructible<Type>::value)
            std::move(data_ + last_position, data_ + size(), data_ + first_position);
        else
            std::copy(data_ + last_position, data_ + size(), data_ + first_position);

        size_ -= difference;

        return (last_equals_end)
            ? iterator(data_ + last_position)
            : iterator(data_ + first_position);
    }

    constexpr void pop_back() noexcept
    {
        std::allocator_traits<allocator_type>::destroy(allocator_, data_ + size() - 1);
        size_ -= 1;
    }

    constexpr void resize(size_type count, const_reference value = value_type())
    {
        auto temp_size = size();

        if (count < size()) {
            for (size_type index{ count }; index < temp_size; ++index)
                pop_back();
        }
        else {
            if (count > capacity())
                reallocate_strong_guarantee(count);

            for (size_type index{ temp_size }; index < count; ++index)
                insert_end_strong_guarantee(value);
        }
    }

    template<typename...Args>
    constexpr iterator emplace(const iterator pos, Args&&...args)
    {
        static_assert(pos <= end() && "Vector's argument out of range");
        size_type pos_index_position = std::distance(pos, begin());

        if (size() + 1 < capacity()) {
            if (pos == end()) {
                try {
                    std::allocator_traits<allocator_type>::construct(
                        allocator_,
                        data_ + size(),
                        std::forward<Args>(args)...);
                }
                catch (...) {
                    std::allocator_traits<allocator_type>::destroy(allocator_, data_ + size());
                    throw;
                }
                ++size_;
            }
            else {
                shift_and_construct(pos_index_position, std::forward<Args>(args)...);
            }
        }
        else {
            do {
                if (capacity_ == 0)
                    capacity_ = 1;

                capacity_ *= realloc_factor();
            } while (capacity_ < 1 + size());

            reallocate_strong_guarantee(capacity_);

            // Checks if move constructor is noexcept otherwise does copy.
            shift_and_construct(pos_index_position, std::forward<Args>(args)...);
        }
        return iterator(data_ + pos_index_position);
    }

    template<typename...Args>
    constexpr reference emplace_back(Args...args)
    {
        emplace(end(), std::forward<Args>(args)...);
        return *(data_ + size() - 1);
    }

    constexpr void push_back(const Type& value)
    {
        emplace_back(value);
    }

    constexpr void push_back(Type&& value)
    {
        emplace_back(std::move(value));
    }

    constexpr void swap(vector& other) noexcept
    {
        if (this == &other) return;

        if (std::allocator_traits<allocator_type>::propagate_on_container_swap::value ||
            std::allocator_traits<allocator_type>::is_always_equal::value)
        {
            std::swap(allocator_, other.allocator_);
        }
        std::swap(data_, other.data_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }

private:
    constexpr size_type realloc_factor()
    {
        return 2;
    }

    constexpr void reset(vector& other) noexcept
    {
        other.data_ = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;
    }

    constexpr void allocate(size_type capacity)
    {
        capacity_ = capacity;
        data_ = std::allocator_traits<allocator_type>::allocate(allocator_, capacity);
    }

    constexpr void deallocate(size_type capacity)
    {
        std::allocator_traits<allocator_type>::deallocate(allocator_, data_, capacity);
        capacity_ = 0;
        size_ = 0;
    }

    constexpr void reallocate(size_type old_cap, size_type new_cap)
    {
        deallocate(old_cap);
        allocate(new_cap);
    }

    constexpr void construct(size_type size, const Type& value)
    {
        size_ = size;
        for (size_type index = 0; index < size; ++index)
            std::allocator_traits<allocator_type>::construct(allocator_, data_ + index, value);
    }

    constexpr void destruct(size_type size)
    {
        for (size_type index = 0; index < size; ++index)
            std::allocator_traits<allocator_type>::destroy(allocator_, data_ + index);
        size_ = 0;
    }

    constexpr void allocate_and_copy_construct(
        size_type capacity,
        size_type size,
        const Type& value = Type())
    {
        allocate(capacity);
        construct(size, value);
    }

    constexpr void construct_init_list(std::initializer_list<Type> values)
    {
        size_ = values.size();

        for (size_type index = 0; const auto & currentValue : values){
            std::allocator_traits<allocator_type>::construct(
                allocator_,
                data_+(index++),
                currentValue);
        }
    }

    constexpr void deallocate_and_destruct(size_type capacity, size_type size)
    {
        destruct(size);
        deallocate(capacity);
    }

    constexpr void deallocate_destruct_keep_size_and_capacity(
        size_type size,
        size_type capacity)
    {
        for (size_type index = 0; index < size_; ++index)
            std::allocator_traits<allocator_type>::destroy(allocator_, data_ + index);
        std::allocator_traits<allocator_type>::deallocate(allocator_, data_, capacity_);

        capacity_ = capacity;
        size_ = size;
    }

    constexpr void uninitialized_alloc_copy(const vector& other)
    {
        size_ = other.size_;
        for (size_type index = 0; index < size_; ++index) {
            std::allocator_traits<allocator_type>::construct(
                allocator_,
                data_ + index,
                *(other.data_ + index));
        }
    }

    constexpr void uninitialized_alloc_move(vector&& other) noexcept
    {
        size_ = other.size_;
        capacity_ = other.capacity_;
        for (size_type index = 0; index < size_; ++index) {
            std::allocator_traits<allocator_type>::construct(
                allocator_,
                data_ + index,
                std::move(*(other.data_ + index)));
        }

        reset(other);
    }

    constexpr void copy(const vector& other)
    {
        if (other.data_) {
            allocate(other.size_);
            uninitialized_alloc_copy(other);
        }
        else {
            data_ = nullptr;
        }
    }

    constexpr void reallocate_strong_guarantee(size_type capacity)
    {
        Type* temp = std::allocator_traits<allocator_type>::allocate(allocator_, capacity);

        if (std::is_nothrow_move_constructible<Type>::value ||
            (!std::is_nothrow_move_constructible<Type>::value &&
                !std::is_copy_constructible<Type>::value))
        {
            for (size_type index = 0; index < size_; ++index) {
                std::allocator_traits<allocator_type>::construct(
                    allocator_,
                    temp + index,
                    std::move(data_[index]));
            }
        }
        else {
            size_type copiesMade = 0;
            try {
                for (size_type index = 0; index < size_; ++index) {
                    std::allocator_traits<allocator_type>::construct(
                        allocator_,
                        temp + index,
                        *(data_ + index));

                    ++copiesMade;
                }
            }
            catch (...) {
                for (size_type index = 0; index < copiesMade; ++index)
                    std::allocator_traits<allocator_type>::destroy(allocator_, temp + index);

                std::allocator_traits<allocator_type>::deallocate(allocator_, temp, capacity);
                throw;
            }
        }
        size_type temp_cap = capacity;
        deallocate_destruct_keep_size_and_capacity(size_, temp_cap);
        data_ = temp;
    }

    constexpr void shift_and_construct(
        size_type index_pos,
        const Type& value,
        size_type count = 1)
    {
        size_type copies_made = 0;
        size_type copies_made1 = 0;
        size_type copies_made2 = 0;

        auto count_after_last_element = data_ + size() + count;
        auto last_element = data_ + size();
        auto current_pos = data_ + index_pos;

        try {
            for (size_type index = 0; index < count; ++index) {
                std::allocator_traits<allocator_type>::construct(
                    allocator_,
                    data_ + size() + index,
                    data_[size() + index]);

                ++copies_made;
            }
        }
        catch (...) {
            for (size_type index = 0; index < copies_made; ++index)
                std::allocator_traits<allocator_type>::destroy(allocator_, data_ + size() + index);
            throw;
        }

        try {
            while (current_pos++ != data_ + size()) {
                *(--(count_after_last_element)) = *(--(last_element));
                ++copies_made1;
            }
        }
        catch (...) {
            while (copies_made1 != 0) {
                std::allocator_traits<Allocator>::destroy(
                    allocator_,
                    data_ + size() + 1 + copies_made1);

                --copies_made1;
            }
            throw;
        }

        for (size_type index = 0; index < count; ++index)
            std::allocator_traits<allocator_type>::destroy(allocator_, data_ + index_pos + index);

        try {
            for (size_type index = 0; index < count; ++index) {
                std::allocator_traits<allocator_type>::construct(
                    allocator_,
                    data_ + index_pos + index, value);

                ++copies_made2;
            }
        }
        catch (...) {
            for (size_type index = 0; index < copies_made2; ++index)
                std::allocator_traits<allocator_type>::destroy(allocator_, data_ + size() + index);

            throw;
        }
        size_ += count;
    }

    constexpr void shift_and_construct_init(
        size_type pos_index_position,
        std::initializer_list<Type> list)
    {
        for (size_type index = 0; auto value : list) {
            std::allocator_traits<allocator_type>::construct(
                allocator_,
                data_ + size() + index,
                data_[size() + index]);
            ++index;
        }

        std::copy_backward(
            data_ + pos_index_position,
            data_ + size(),
            data_ + size() + list.size());

        for (size_type index = 0; auto value : list) {
            std::allocator_traits<allocator_type>::destroy(
                allocator_,
                data_ + pos_index_position + index);

            std::allocator_traits<allocator_type>::construct(
                allocator_,
                data_ + pos_index_position + index, value);

            ++index;
        }
        size_ += list.size();
    }

    constexpr void shift_and_construct(size_type index_pos, Type&& value)
    {
        if (std::is_nothrow_move_constructible<Type>::value ||
            (!std::is_nothrow_move_constructible<Type>::value &&
                !std::is_copy_constructible<Type>::value))
        {
            // auto one_after_last_element = data_ + size() + 1;
            // auto last_element = data_ + size();
            // auto current_pos = data_ + index_pos;

            std::allocator_traits<allocator_type>::construct(
                allocator_,
                data_ + size(),
                std::move(*(data_+size())));

            std::move_backward(data_ + index_pos, data_ + size(), data_ + size() + 1);

            std::allocator_traits<allocator_type>::construct(
                allocator_,
                data_ + index_pos,
                std::move(value));

            size_ += 1;
        }
        else {
            shift_and_construct(index_pos, value);
        }
    }

    constexpr void insert_end_strong_guarantee(const Type& value) {
        try {
            std::allocator_traits<allocator_type>::construct(allocator_, data_ + size_, value);
        }
        catch (...) {
            std::allocator_traits<allocator_type>::destroy(allocator_, data_ + size_);
            throw;
        }
        size_ += 1;
    }

    constexpr void  insert_end_strong_guarantee(Type&& value) {
        if constexpr (std::is_nothrow_move_constructible<Type>::value) {
            std::allocator_traits<allocator_type>::construct(
                allocator_,
                data_ + size_,
                std::move(value));
        }
        else {
            insert_end_strong_guarantee(value);
        }
        size_ += 1;
    }
};

// Erasing -----------------------------------------------------------------------------------------

template<typename Type, typename Allocator, typename Val>
constexpr auto erase(vector<Type, Allocator>& vec, const Val& value)
{
    auto iter = std::remove(vec.begin(), vec.end(), value);
    auto dist = std::distance(iter, vec.end());

    vec.erase(iter, vec.end());
    return dist;
}

template<typename Type, typename Allocator, typename Predicate>
constexpr auto erase_if(vector<Type, Allocator>& vec, Predicate predicate)
{
    auto iter = std::remove_if(vec.begin(), vec.end(), predicate);
    auto dist = std::distance(iter, vec.end());

    vec.erase(iter, vec.end());
    return dist;
}

// Vector with polymorphic allocator.
namespace pmr {
    template <class T>
    using vector = vector<T, std::pmr::polymorphic_allocator<T>>;
}

} // namespace wingmann::containers

#endif