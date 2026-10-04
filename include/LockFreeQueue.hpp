#ifndef LOCK_FREE_QUEUE_HPP
#define LOCK_FREE_QUEUE_HPP

#pragma once
#include <atomic>
#include <cassert>
#include <vector>

namespace Trading {

template <typename T, size_t Capacity = 65536>
class LockFreeQueue {
    /// Static capacity assertion to ensure it's a power of 2 for efficient masking
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2 for fast masking.");

private:
    std::vector<T> m_buffer;

    /// Align head and tail to cache line boundaries to prevent false sharing
    alignas(64) std::atomic<size_t> m_head{0};
    alignas(64) std::atomic<size_t> m_tail{0};

    /// Mask for wrapping around the buffer index
    static constexpr size_t Mask = Capacity - 1;

public:
    LockFreeQueue() : m_buffer(Capacity) {
    }

    /// Put item in queue, returns false if the queue is full
    bool enqueue(const T& item) {
        const size_t current_tail = m_tail.load(std::memory_order_relaxed);
        const size_t current_head = m_head.load(std::memory_order_acquire);

        if ((current_tail - current_head) >= Capacity) {
            return false;
        }

        /// Place the item in the buffer and update the tail
        m_buffer[current_tail & Mask] = item;
        m_tail.store(current_tail + 1, std::memory_order_release);
        return true;
    }

    /// Get item from queue, returns false if the queue is empty
    bool dequeue(T& item) {
        const size_t current_head = m_head.load(std::memory_order_relaxed);
        const size_t current_tail = m_tail.load(std::memory_order_acquire);

        if (current_head == current_tail) {
            return false;
        }

        item = m_buffer[current_head & Mask];
        m_head.store(current_head + 1, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return m_head.load(std::memory_order_relaxed) == m_tail.load(std::memory_order_relaxed);
    }
};

}  // namespace Trading

#endif
