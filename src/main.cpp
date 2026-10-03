#include "LockFreeQueue.hpp"
#include "Order.hpp"
#include "RiskManager.hpp"
#include "OrderBook.hpp"
#include "Benchmark.hpp"
#include <thread>
#include <atomic>
#include <iostream>

using namespace Trading;

constexpr size_t TARGET_ORDER_COUNT = 20'000'000;

std::atomic<bool> engineRunning{true};
std::atomic<size_t> processedCount{0};

/// Queues for pipeline stages
LockFreeQueue<Order> marketToRiskQueue;
LockFreeQueue<Order> riskToEngineQueue;
LockFreeQueue<TimingPoint> engineToMetricsQueue;

PerformanceBenchmarker benchmarker;

/// Simulating market data feed
void marketDataProducer() {
    for (size_t i = 1; i <= TARGET_ORDER_COUNT; ++i) {
        Order order;
        order.id = i;

        /// Timestamp is generated in nanoseconds for high-resolution latency measurement
        order.timestamp = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
            ).count()
        );

        order.price = 100.0 + (i % 10);
        order.quantity = 10 + (i % 100);
        order.side = (i % 2 == 0) ? Side::BUY : Side::SELL;

        while (!marketToRiskQueue.enqueue(order)) {
            std::this_thread::yield();
        }
    }
}

/// Validates orders and forwards accepted ones
void riskEngineConsumer() {
    RiskManager riskManager;
    Order order;
    
    while (engineRunning || !marketToRiskQueue.empty()) {
        if (marketToRiskQueue.dequeue(order)) {
            RiskResult res = riskManager.validateOrder(order);
            if (res == RiskResult::ACCEPTED) {
                while (!riskToEngineQueue.enqueue(order)) {
                    std::this_thread::yield();
                }
            } else {
                processedCount++;
            }
        } else {
            std::this_thread::yield();
        }
    }
}

/// Process validated orders and record latency
void matchingEngineConsumer() {
    OrderBook orderBook(1); 
    Order order;
    
    while (engineRunning || !riskToEngineQueue.empty()) {
        if (riskToEngineQueue.dequeue(order)) {
            TimingPoint tp;
            tp.start = std::chrono::high_resolution_clock::now();
            
            /// Process the order in the order book
            orderBook.addOrder(order);
            
            tp.end = std::chrono::high_resolution_clock::now();

            /// Forward timing point to metrics queue
            while (!engineToMetricsQueue.enqueue(tp)) {
                std::this_thread::yield();
            }
        } else {
            std::this_thread::yield();
        } 
    }
}

/// Collects metrics and generates a performance report
void metricsConsumer() {
    TimingPoint tp;
    while (engineRunning || !engineToMetricsQueue.empty()) {
        if (engineToMetricsQueue.dequeue(tp)) {
            benchmarker.recordLatency(tp.start, tp.end);
            processedCount++;
        } else {
            std::this_thread::yield();
        }
    }
}


int main() {
    std::cout << "Initializing 4-Stage Asynchronous HFT Pipeline\n";

    benchmarker.reserve(TARGET_ORDER_COUNT);
    auto start_time = std::chrono::high_resolution_clock::now();

    /// Creating threads for each stage
    std::thread thread4(metricsConsumer);
    std::thread thread3(matchingEngineConsumer);
    std::thread thread2(riskEngineConsumer);
    std::thread thread1(marketDataProducer);

    thread1.join();
    
    /// Wait for all orders to be processed before shutting down the engine
    while (processedCount < TARGET_ORDER_COUNT) {
        std::this_thread::yield();
    }

    engineRunning = false;
    thread2.join();
    thread3.join();
    thread4.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time).count();

    benchmarker.generateReport(duration, TARGET_ORDER_COUNT);

    return 0;
}
