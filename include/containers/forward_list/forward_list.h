/// @file   forward_list.h
/// @author Alexander Shavrov (alexander.shavrov@outlook.com)
/// @brief  Forward list data structure implementation.
/// @date   2022-10-13
///
/// @copyright Copyright (c) 2022
///
/// This file is distributed under the MIT License.
/// See LICENSE file for details.
///

#ifndef WINGMANN_CONTAINERS_FORWARD_LIST_H
#define WINGMANN_CONTAINERS_FORWARD_LIST_H

#include "forward_list_node.h"

#include <iterators/forward_iterator.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <concepts>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <utility>

namespace wingmann::containers {

template<typename T>
class forward_list {
public:
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = T*;
    using const_pointer   = const pointer;
    using node_type       = types::node<T>;
    using node_pointer    = node_type*;
    using iterator        = wingmann::iterators::forward_iterator<value_type, node_type>;
    using const_iterator  = wingmann::iterators::forward_iterator<const value_type, node_type>;

private:
    node_pointer head_;
    node_pointer tail_;
    size_type size_{};

public:
    constexpr forward_list() noexcept : head_{nullptr}, tail_{nullptr}, size_{}
    {
    }

    constexpr explicit forward_list(size_type count, const_reference value) :size_{count}
    {
        auto current_node = new node_type{value};
        head_ = current_node;

        for (size_type index = 0; index < count - 1; ++index) {
            current_node->next = new node_type{value};
            current_node = current_node->next;
        }
        tail_ = current_node;
        tail_->next = nullptr;
    }

    constexpr explicit forward_list(size_type count) : forward_list(count, T{})
    {
    }

    template<std::input_iterator input_iter>
    constexpr forward_list(input_iter first, input_iter last)
        : size_{static_cast<size_type>(std::distance(first, last))}
    {
        auto current_node = new node_type{*first};
        head_ = current_node;

        for (size_type index = 0; index < size_ - 1; ++index) {
            current_node->next = new types::node<T>(*(++first));
            current_node = current_node->next;
        }
        tail_ = current_node;
        tail_->next = nullptr;
    }

    constexpr forward_list(const forward_list& other)
    {
        if (other.head_) {
            size_ = other.size_;

            auto current_node = new node_type{other.head_->data};
            auto current_other_node = other.head_;

            head_ = current_node;

            while (current_other_node->next != nullptr) {
                current_node->next = new node_type{current_other_node->next->data};
                current_node = current_node->next;
                current_other_node = current_other_node->next;
            }
            tail_ = current_node;
            tail_->next = nullptr;
        }
        else {
            forward_list{};
        }
    }

    constexpr forward_list(forward_list&& other) noexcept : forward_list()
    {
        other.swap(*this);
    }

    constexpr forward_list(std::initializer_list<T> list) : size_{list.size()}
    {
        auto current_node = new node_type{*(list.begin())};
        head_ = current_node;

        for (auto it = list.begin() + 1; it != list.end(); ++it) {
            current_node->next = new node_type{*it};
            current_node = current_node->next;
        }
        tail_ = current_node;
        tail_->next = nullptr;
    }

    ~forward_list()
    {
        auto current_node = head_;

        while (current_node != nullptr) {
            auto next_node = current_node->next;

            delete current_node;
            current_node = next_node;
        }
        head_ = nullptr;
    }

    // Assignment operator -------------------------------------------------------------------------

    constexpr forward_list& operator=(const forward_list& other) {
        forward_list temp_list(other);
        temp_list.swap(*this);
        return *this;
    }

    constexpr forward_list& operator=(forward_list&& other) noexcept
    {
        other.swap(*this);
        deallocate(other);

        return *this;
    }

    constexpr forward_list& operator=(std::initializer_list<T> list)
    {
        forward_list temp_list{list};
        temp_list.swap(*this);

        return *this;
    }

    // Other ---------------------------------------------------------------------------------------

    constexpr void assign(size_type new_size, const_reference value)
    {
        deallocate(*this);
        forward_list temp_list(new_size, value);
        temp_list.swap(*this);
    }

    constexpr void assign(std::initializer_list<T> list)
    {
        deallocate(*this);
        forward_list temp_list{list};
        temp_list.swap(*this);
    }

    template<typename input_iter>
    constexpr void assign(input_iter first, input_iter last)
    {
        deallocate(*this);
        forward_list temp_list(first, last);
        temp_list.swap(*this);
    }

    // Element access ------------------------------------------------------------------------------

    constexpr reference front() noexcept
    {
        return head_->data;
    }

    constexpr const_reference front() const noexcept
    {
        return head_->data;
    }

    // Iterators -----------------------------------------------------------------------------------

    constexpr iterator begin() noexcept
    {
        return iterator(head_);
    }

    constexpr const_iterator begin() const noexcept
    {
        return const_iterator(head_);
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return const_iterator(head_);
    }

    constexpr iterator end() noexcept
    {
        if (tail_ == nullptr)
            return nullptr;

        return iterator(tail_->next);
    }

    constexpr const_iterator end() const noexcept
    {
        if (tail_ == nullptr)
            return nullptr;

        return const_iterator(tail_->next);
    }

    constexpr const_iterator cend() const noexcept
    {
        if (tail_ == nullptr)
            return nullptr;

        return const_iterator(tail_->next);
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept
    {
        return size_ == 0;
    }

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

    // Modifiers -----------------------------------------------------------------------------------

    constexpr void clear() noexcept
    {
        deallocate(*this);
    }

    template<typename...Args>
    constexpr iterator emplace_after(const iterator position, Args...args)
    {
        auto temp = position.get_node_address();
        auto current_node = new node_type{std::forward<Args>(args)...};

        if (temp == tail_) {
            tail_->next = current_node;
            current_node->next = nullptr;
            tail_ = tail_->next;
        }
        else {
            auto next_temp = position.get_node_next_address();
            temp->next = current_node;
            current_node->next = next_temp;
        }
        size_ += 1;

        return iterator(current_node);
    }

    constexpr iterator insert_after(const iterator position, const_reference value)
    {
        return emplace_after(position, value);
    }

    constexpr iterator insert_after(const iterator position, T&& value)
    {
        return emplace_after(position, std::move(value));
    }

    constexpr iterator insert_after(const iterator position, size_type count, const_reference value)
    {
        iterator temp;

        for (size_type i = 0; i < count; ++i)
            temp = emplace_after(position, value);

        return (count == 0) ? position : temp;
    }

    constexpr iterator insert_after(const iterator position, std::initializer_list<T> list)
    {
        iterator temp;

        for (auto current : list)
            temp = emplace_after(position, current);

        return (list.size() == 0) ? position : temp;
    }

    constexpr iterator erase_after(const iterator position)
    {
        auto temp = position.get_node_address();
        auto next_temp = temp->next;

        if (temp != nullptr && next_temp->next == nullptr) {
            temp->next = nullptr;
        }
        else if (temp != nullptr) {
            temp->next = next_temp->next;
            delete next_temp;
        }
        size_ -= 1;
        auto pos = position;

        return (++pos != nullptr) ? pos : end();
    }

    constexpr iterator erase_after(iterator first, iterator last)
    {
        auto firstNode_temp = first.get_node_address();
        auto first_node_next = first.get_node_next_address();
        auto last_node_temp = last.get_node_address();

        while (first_node_next != last_node_temp) {
            auto temp = first_node_next->next;

            delete first_node_next;
            first_node_next = temp;

            --size_;
        }
        firstNode_temp->next = last_node_temp;

        return last;
    }

    constexpr void push_front(const_reference value)
    {
        emplace_front(value);
    }

    constexpr void push_front(T&& value)
    {
        emplace_front(std::move(value));
    }

    template<typename...Args>
    constexpr reference emplace_front(Args...args)
    {
        auto head_temp = head_;
        auto current = new node_type{std::forward<Args>(args)...};

        head_ = current;
        head_->next = head_temp;

        ++size_;
        return head_->data;
    }

    constexpr void pop_front()
    {
        auto head_temp = head_;
        auto next_temp = head_->next;

        head_ = next_temp;
        delete head_temp;

        --size_;
    }

    constexpr void resize(size_type count, const_reference value= T{})
    {
        if (count < size()) {
            auto temp = head_;

            for (size_type index = 0; index < count - 1; ++index)
                temp = temp->next;

            auto other_temp = temp;
            temp = temp->next;

            while (temp->next != tail_) {
                auto temp_node = temp->next;
                delete temp;
                temp = temp_node;
            }
            delete tail_;

            tail_ = other_temp;
            tail_->next = nullptr;
        }

        else {
            auto temp_tail = tail_;

            for (size_type index = 0; index < count; ++index) {
                temp_tail->next = new node_type{value};
                temp_tail = temp_tail->next;
            }
            tail_ = temp_tail;
            tail_->next = nullptr;
        }
        size_ = count;
    }

    constexpr void swap(forward_list& other) noexcept
    {
        auto temp_node = head_;
        head_ = other.head_;
        other.head_ = temp_node;

        auto temp_tail = tail_;
        tail_ = other.tail_;
        other.tail_ = temp_tail;

        std::swap(size_, other.size_);
    }

    constexpr void splice_after(const iterator position, forward_list& other)
    {
        auto current_pos = position.get_node_address();
        if (current_pos == tail_) {
            tail_->next = other.head_;
            tail_ = other.tail_;
        }
        else {
            auto next_node = position.get_node_next_address();
            auto next_next = next_node->next;
            auto temp_head = other.head_;
            auto temp_tail = other.tail_;

            current_pos->next = temp_head;
            temp_tail->next = next_next;
        }
        size_ += other.size_;

        other.size_ = 0;
        other.head_ = nullptr;
        other.tail_ = nullptr;
    }

    constexpr size_type remove(const_reference to_remove_value)
    {
        return remove_if([&to_remove_value](auto other) { return other == to_remove_value; });
    }

    template<typename Predicate>
    constexpr size_type remove_if(Predicate pred)
    {
        auto before_begin = new node_type{};
        auto temp = before_begin;

        before_begin->next = head_;
        size_type removed{};

        for (node_pointer first = head_; first != nullptr;)
        {
            if (pred(first->data)) {
                if (first == head_) {
                    first = remove(before_begin);
                    head_ = first;
                }
                else {
                    first = remove(before_begin);
                }
                ++removed;
            }
            else {
                before_begin = first;
                first = first->next;
            }
        }
        delete temp;
        size_ -= removed;

        return removed;
    }

    constexpr size_type unique()
    {
        size_type removed{};

        for (node_pointer first = head_; first != nullptr;) {
            if (first == tail_) break;

            if (first->data == first->next->data) {
                first = remove(first);
                ++removed;
            }
            else {
                first = first->next;
            }
        }
        size_ -= removed;
        return removed;
    }

    constexpr void reverse() noexcept
    {
        auto temp_data = head_;
        auto temp = forward_list{};

        while (temp_data != nullptr) {
            temp.push_front(temp_data->data);
            temp_data = temp_data->next;
        }
        temp.swap(*this);
    }

    constexpr void sort()
    {
        node_pointer sorted = nullptr;
        node_pointer current = head_;

        while (current != nullptr) {
            node_pointer next = current->next;
            sort(sorted, current);
            current = next;
        }
        head_ = sorted;
    }

    constexpr bool operator<=(const forward_list<T>& other)
    {
        return other >= *this;
    }

    constexpr bool operator >=(const forward_list<T>& other)
    {
        return *this >= other;
    }

private:
    constexpr node_pointer remove(node_pointer before_node) noexcept
    {
        const auto to_remove = before_node->next;
        const auto removed_next = to_remove->next;
        before_node->next = removed_next;

        delete to_remove;
        return removed_next;
    }

    constexpr void sort(node_pointer (&head_ref), node_pointer new_node)
    {
        node_pointer current;

        if (head_ref == nullptr || (head_ref)->data >= new_node->data) {
            new_node->next = head_ref;
            head_ref = new_node;
        }
        else {
            current = head_ref;

            while (current->next != nullptr && current->next->data < new_node->data)
                current = current->next;

            new_node->next = current->next;
            current->next = new_node;
        }
    }

    constexpr void deallocate(forward_list& other) noexcept
    {
        if (!other.head_)
            return;

        node_pointer current_node = other.head_;

        while (current_node != nullptr) {
            node_pointer next_node = current_node->next;
            delete current_node;
            current_node = next_node;
        }
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }
};

template<typename T>
constexpr bool operator==(const forward_list<T>& first, const forward_list<T>& other)
{
    auto temp = first.head_;
    bool not_equal = false;

    if (first.size() != other.size()) {
        return false;
    }
    else {
        while (temp != nullptr) {
            if (!(temp->data == other->data)) {
                not_equal = true;
                break;
            }
            temp = temp->next;
        }
    }
    return !not_equal;
}

template<typename T>
constexpr bool operator!= (const forward_list<T>& first, const forward_list<T>& other)
{
    return first != other;
}

template<typename T>
constexpr bool operator<(const forward_list<T>& first, const forward_list<T>& other)
{
    return (std::lexicographical_compare(first.begin(), first.end(), other.begin(), other.end()));
}

template<typename T>
constexpr bool operator>(const forward_list<T>& first, const forward_list<T>& other)
{
    return !(std::lexicographical_compare(first.begin(), first.end(), other.begin(), other.end()));
}

} // namespace wingmann::containers

#endif // WINGMANN_CONTAINERS_FORWARD_LIST_H
