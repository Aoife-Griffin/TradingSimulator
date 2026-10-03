#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace Trading {

template<typename T>
class MutexQueue {
private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;
    bool m_shutdown{false};

public:
    MutexQueue() = default;

    /// No copies or duplicates
    MutexQueue(const MutexQueue&) = delete;
    MutexQueue& operator=(const MutexQueue&) = delete;

    /// Add an item to the queue
    void enqueue(const T& item) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(item);
        }

        /// Alert a waiting user
        m_condition.notify_one();
    }

    /// Wait until an item is available or the queue is shut down.
    bool dequeue(T& item) {
        std::unique_lock<std::mutex> lock(m_mutex);

        m_condition.wait(lock, [this] {
            return !m_queue.empty() || m_shutdown;
        });

        /// If shutdown happens and queue is empty, false is returned -> no more items to process
        if (m_queue.empty()) {
            return false;
        }

        /// If not, pop the front item and return true
        item = std::move(m_queue.front());
        m_queue.pop();

        return true;
    }

    /// Tell users that processing is done
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_shutdown = true;
        }

        /// Tell all consumers they can exit.
        m_condition.notify_all();
    }

    /// Check if queue is empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    /// Return queue size
    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
};

}