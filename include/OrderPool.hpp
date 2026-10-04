#ifndef ORDER_POOL_HPP
#define ORDER_POOL_HPP
#pragma once
#include <iostream>
#include "Order.hpp"
#include <vector>
#include <cassert>

namespace Trading {

template<size_t PoolSize = 5000000>
class OrderPool {
public:
    /// A node in the pool that contains an order and a pointer to the next free node
    struct Node {
        Order order;
        int next_free_idx{-1}; 
    };

private:
    /// Pool of nodes pre-allocated to avoid dynamic memory allocation during trading
    std::vector<Node> m_pool;
    int m_next_available_slot{0};

public:
    OrderPool() : m_pool(PoolSize) {
        /// Connects the nodes in a free list in o(1) linear time
        for (size_t i = 0; i < PoolSize - 1; ++i) {
            m_pool[i].next_free_idx = static_cast<int>(i + 1);
        }
        m_pool[PoolSize - 1].next_free_idx = -1; // End of list
    }

    /// Allocate a node from the pool in Linear time
    int allocate(const Order& order) {
        if (m_next_available_slot == -1) {
            std::cerr << "[CRITICAL ERROR] OrderPool has run out of pre-allocated slots!";
            assert(false);
            return -1;
        }

        int allocated_idx = m_next_available_slot;
        
        /// Move the next available slot to the next free node in the pool
        m_next_available_slot = m_pool[allocated_idx].next_free_idx;

        /// Fill the selected node with the order and detach it from the free list
        m_pool[allocated_idx].order = order;
        m_pool[allocated_idx].next_free_idx = -1; 

        return allocated_idx;
    }

    /// Return a node back to the pool to the front of the list
    void deallocate(int index) {
        m_pool[index].next_free_idx = m_next_available_slot;
        m_next_available_slot = index;
    }

    /// Accessors for the order at a given index
    Order& get(int index) { return m_pool[index].order; }
    const Order& get(int index) const { return m_pool[index].order; }
};

}
#endif
