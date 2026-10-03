#include "LockFreeQueue.hpp"
#include "MutexQueue.hpp"
#include "Order.hpp"
#include <chrono>
#include <iostream>
#include <thread>
#include <string>

using namespace Trading;

constexpr size_t NUM_ORDERS = 5'000'000; 

template<typename QueueType>
void benchmarkQueue(const std::string& queueName) {
    QueueType queue;
    std::atomic<bool> producerDone{false};
    size_t consumedOrders = 0;
    
    if constexpr (std::is_same_v<QueueType, MutexQueue<Order>>) {
        
    }

    auto start = std::chrono::steady_clock::now();

    /// Producer Thread
    std::thread producer([&]() {
        for (size_t i = 0; i < NUM_ORDERS; ++i) {
            Order order;
            order.id = i;
            order.price = 100.0;
            order.quantity = 10;
            order.side = Side::BUY;
            
            /// If using LockFreeQueue, yield until space is available; MutexQueue will block internally
            if constexpr (std::is_same_v<QueueType, LockFreeQueue<Order>>) {
                while (!queue.enqueue(order)) {
                    std::this_thread::yield();
                }
            } else {
                queue.enqueue(order); 
            }
        }
        producerDone.store(true, std::memory_order_release);
        
        /// If using MutexQueue, shutdown to unblock
        if constexpr (std::is_same_v<QueueType, MutexQueue<Order>>) {
            queue.shutdown(); 
        }
    });

    /// Consumer Thread
    std::thread consumer([&]() {
        Order order;
        while (!producerDone.load(std::memory_order_acquire) || consumedOrders < NUM_ORDERS) {
            if (queue.dequeue(order)) {
                ++consumedOrders;
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    /// Calculate elapsed time and throughput
    auto end = std::chrono::steady_clock::now();
    const auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    const double seconds = elapsedUs / 1'000'000.0;
    const double throughput = NUM_ORDERS / seconds;


    std::cout << "  Total Time:  " << elapsedUs << " us (" << seconds << " s)\n";
    std::cout << "  Throughput:  " << throughput << " orders/sec";
}

int main() {
    std::cout << "Total testing operations: " << NUM_ORDERS << "\n";

    /// Use lock approach first
    benchmarkQueue<MutexQueue<Order>>("Mutex-Based Blocking Queue");

    /// Run hardware approach second
    benchmarkQueue<LockFreeQueue<Order>>("Lock-Free Ring Buffer");

    return 0;
}
