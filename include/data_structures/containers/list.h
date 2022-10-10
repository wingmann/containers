/// @file   list.h
/// @author Alexander Shavrov (alexander.shavrov@outlook.com)
/// @brief  Doubly linked list implementation.
/// @date   2022-10-02
///
/// @copyright Copyright (c) 2022
///
/// This file is distributed under the MIT License.
/// See LICENSE file for details.
///

#ifndef WINGMANN_DATA_STRUCTURES_CONTAINERS_LIST_H
#define WINGMANN_DATA_STRUCTURES_CONTAINERS_LIST_H

#include <cstddef>

namespace wingmann::containers {

/// @brief Doubly linked list.
/// @tparam T Type of list elements.
///
/// @see https://en.wikipedia.org/wiki/Doubly_linked_list
///
template<typename T>
class list {
    struct node {
        T info;
        node* next;
        node* back;
    };

public:
    using node_type = node;

protected:
    std::size_t size_{};
    node* first_{};
    node* last_{};

public:
    list() = default;
    list(const list<T>& other);
    virtual ~list();

public:
    list<T>& operator=(const list<T>&);

public:
    void reset();

    [[nodiscard]] bool is_empty() const;

    void destroy();

    [[nodiscard]] std::size_t size() const;

    T front() const;
    T back() const;

    bool search(const T& search_item) const;

    void insert(const T& insert_item);

    void delete_node(const T& delete_item);

private:
    void copy(const list<T>& other);
};

template<typename T>
list<T>::list(const list<T>& other)
{
    first_ = nullptr;
    copy(other);
}

template<typename T>
list<T>::~list()
{
    destroy();
}

template<typename T>
list<T>& list<T>::operator=(const list<T>& other)
{
    if (this != &other)
        copy(other);

    return *this;
}

template<typename T>
void list<T>::reset()
{
    destroy();
}

template<typename T>
bool list<T>::is_empty() const
{
    return first_ == nullptr;
}

template<typename T>
void list<T>::destroy()
{
    node* temp;

    while (first_ != nullptr) {
        temp = first_;
        first_ = first_->next;
        delete temp;
    }
    last_ = nullptr;
    size_ = 0;
}

template<typename T>
std::size_t list<T>::size() const
{
    return size_;
}

template<typename T>
bool list<T>::search(const T& search_item) const
{
    bool found = false;
    node* current;

    current = first_;

    while ((current != nullptr) && !found) {
        if (current->info >= search_item)
            found = true;
        else
            current = current->next;
    }

    if (found)
        found = (current->info == search_item);

    return found;
}

template<typename T>
T list<T>::front() const
{
    return first_->info;
}

template<typename T>
T list<T>::back() const
{
    return last_->info;
}

template<typename T>
void list<T>::insert(const T& insert_item)
{
    node* current;
    node* trail_current = nullptr;
    node* new_node;
    bool found;

    new_node = new node{};
    new_node->info = insert_item;
    new_node->next = nullptr;
    new_node->back = nullptr;

    if (first_ == nullptr) {
        first_ = new_node;
        last_ = new_node;
        size_++;
    }
    else {
        found = false;
        current = first_;

        while ((current != nullptr) && !found) {
            if (current->info >= insert_item) {
                found = true;
            }
            else {
                trail_current = current;
                current = current->next;
            }
        }

        if (current == first_) {
            first_->back = new_node;
            new_node->next = first_;
            first_ = new_node;
            size_++;
        }
        else {
            if (current != nullptr) {
                trail_current->next = new_node;
                new_node->back = trail_current;
                new_node->next = current;
                current->back = new_node;
            }
            else {
                trail_current->next = new_node;
                new_node->back = trail_current;
                last_ = new_node;
            }
            size_++;
        }
    }
}

template<typename T>
void list<T>::delete_node(const T& delete_item) {
    node* current;
    node* trail_current;

    bool found;

    if (first_->info == delete_item) {
        current = first_;
        first_ = first_->next;

        if (first_ != nullptr)
            first_->back = nullptr;
        else
            last_ = nullptr;

        size_--;
        delete current;
    }
    else {
        found = false;
        current = first_;

        while ((current != nullptr) && !found) {
            if (current->info >= delete_item)
                found = true;
            else
                current = current->next;
        }

        if (current->info == delete_item) {
            trail_current = current->back;
            trail_current->next = current->next;

            if (current->next != nullptr)
                current->next->back = trail_current;

            if (current == last_)
                last_ = trail_current;

            size_--;
            delete current;
        }
    }
}

template<typename T>
void list<T>::copy(const list<T>& other)
{
    node* new_node;
    node* current;

    if (first_ != nullptr) destroy();

    if (other.first_ == nullptr) {
        first_ = nullptr;
        last_ = nullptr;
        size_ = 0;
    }
    else {
        current = other.first_;
        size_ = other.count;

        first_ = new node{};
        first_->info = current->info;
        first_->next = nullptr;
        first_->back = nullptr;
        last_ = first_;

        current = current->next;

        while (current != nullptr) {
            new_node = new node{};
            new_node->info = current->info;
            new_node->next = nullptr;
            new_node->back = last_;

            last_->next = new_node;
            last_ = new_node;
            current = current->next;
        }
    }
}

} // namespace wingmann::containers

#endif // WINGMANN_DATA_STRUCTURES_CONTAINERS_LIST_H
