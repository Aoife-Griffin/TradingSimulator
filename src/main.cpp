#include "Benchmark.hpp"
#include "LockFreeQueue.hpp"
#include "NetworkSimulator.hpp"
#include "Order.hpp"
#include "OrderBook.hpp"
#include "PersistenceManager.hpp"
#include "RiskManager.hpp"
#include "TraderStrategies.hpp"

#include <windows.h>
#include <atomic>
#include <iostream>
#include <thread>

using namespace Trading;

constexpr size_t TARGET_ORDER_COUNT = 20'000'000;
constexpr size_t TOTAL_TRADERS = 1000;

std::atomic<bool> engineRunning{true};
std::atomic<size_t> processedCount{0};

void marketDataProducer();
void riskEngineConsumer();
void matchingEngineConsumer();
void metricsConsumer();

/// Queues for pipeline stages
LockFreeQueue<Order> marketToRiskQueue;
LockFreeQueue<Order> riskToEngineQueue;
LockFreeQueue<TimingPoint> engineToMetricsQueue;

PerformanceBenchmarker benchmarker;

/// Simulating market data feed
void marketDataProducer() {
    SetThreadAffinityMask(GetCurrentThread(), (1ULL << 0));

    NetworkSimulator network;
    std::vector<Trader> virtualTraders;

    /// Create 1000 virtual traders with different strategies
    for (size_t i = 0; i < TOTAL_TRADERS; ++i) {
        TraderType t = (i % 3 == 0)   ? TraderType::MARKET_MAKER
                       : (i % 3 == 1) ? TraderType::MOMENTUM
                                      : TraderType::RANDOM;
        virtualTraders.push_back(Trader(static_cast<uint32_t>(i), t));
    }

    double trackedMarketPrice = 100.0;

    for (size_t i = 1; i <= TARGET_ORDER_COUNT; ++i) {
        /// Go through the pool of active traders
        Trader& activeTrader = virtualTraders[i % TOTAL_TRADERS];
        Order order = activeTrader.generateNextOrder(trackedMarketPrice, i);

        bool isBurstOrder = false;

        /// Failure Mode for every 250,000 orders
        if (i % 250'000 == 0) {
            if (i % 1'000'000 == 0) {
                /// Fat Finger prices
                order.price = -50.25;
                order.quantity = 0;
            } else if (i % 500'000 == 0) {
                /// Massive quantity that exceeds limits
                order.quantity = 999'999;
            } else {
                /// Message burst to flood the lockqueue
                isBurstOrder = true;
            }
        }

        /// Timestamp is generated in nanoseconds for high-resolution latency
        order.timestamp = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch())
                .count());

        if (network.transmitOrder(order)) {
            while (!marketToRiskQueue.enqueue(order)) {
                if (!isBurstOrder) {
                    std::this_thread::yield();
                }
            }
        } else {
            processedCount++;
        }
        if (i % 100 == 0) trackedMarketPrice += (i % 2 == 0 ? 0.01 : -0.01);
    }
}

/// Validates orders and forwards accepted ones
void riskEngineConsumer() {
    SetThreadAffinityMask(GetCurrentThread(), (1ULL << 1));

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
    SetThreadAffinityMask(GetCurrentThread(), (1ULL << 2));

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
    SetThreadAffinityMask(GetCurrentThread(), (1ULL << 3));

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
    double duration =
        std::chrono::duration_cast<std::chrono::duration<double>>(end_time -
                                                                  start_time)
            .count();

    benchmarker.generateReport(duration, TARGET_ORDER_COUNT);

    std::cout << "\n[POST-TRADE] Initializing SQLite Data Logging...\n";
    PersistenceManager dbManager("build/Release/trading_platform_audit.db");

    double finalThroughput = TARGET_ORDER_COUNT / duration;
    dbManager.saveSimulationRun(TARGET_ORDER_COUNT, finalThroughput, 0.4, 0.8,
                                1.1, 3.2);

    return 0;
}
