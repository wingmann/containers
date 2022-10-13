#ifndef WINGMANN_CONTAINERS_FORWARD_LIST_FORWARD_LIST_NODE_H
#define WINGMANN_CONTAINERS_FORWARD_LIST_FORWARD_LIST_NODE_H

#include <type_traits>

namespace wingmann::containers::types {

template<typename T>
struct node {
    T data;
    node* next;

public:
    node() = default;

    template<typename...Args>
    constexpr explicit node(Args&&...args) : data{std::forward<Args>(args)...}
    {
    }
};

} // namespace wingmann::containers::types

#endif // WINGMANN_CONTAINERS_FORWARD_LIST_FORWARD_LIST_NODE_H
