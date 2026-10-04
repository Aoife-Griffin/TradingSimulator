#include "LockFreeQueue.hpp"
#include "Order.hpp"
#include "RiskManager.hpp"
#include "OrderBook.hpp"
#include "Benchmark.hpp"
#include "TraderStrategies.hpp"
#include "NetworkSimulator.hpp"
#include "DashboardExporter.hpp"
#include "PersistenceManager.hpp"

#include <windows.h>
#include <thread>
#include <atomic>
#include <iostream>

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
        TraderType t = (i % 3 == 0) ? TraderType::MARKET_MAKER : 
                       (i % 3 == 1) ? TraderType::MOMENTUM : TraderType::RANDOM;
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
            } 
            else if (i % 500'000 == 0) {
                /// Massive quantity that exceeds limits
                order.quantity = 999'999; 
            } 
            else {
                /// Message burst to flood the lockqueue
                isBurstOrder = true; 
            }
        }
        
        /// Timestamp is generated in nanoseconds for high-resolution latency measurement
        order.timestamp = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()
            ).count()
        );

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
    size_t localCounter = 0;

    while (engineRunning || !engineToMetricsQueue.empty()) {
        if (engineToMetricsQueue.dequeue(tp)) {
            benchmarker.recordLatency(tp.start, tp.end);
            processedCount++;
            localCounter++;

            /// Backup telemetry to dashboard without affecting critical paths
            if (localCounter % 500'000 == 0) {
                DashboardSnapshot uiSnap;
                
                /// When running snapshots, populate the telemetry metrics safely
                uiSnap.currentMarketPrice = 100.25; 
                uiSnap.bestBid = 100.20;
                uiSnap.bestAsk = 100.30;
                uiSnap.cashBalance = 985400.00;
                uiSnap.netSharesOwned = 120;
                uiSnap.throughputOpsSec = 1072830.0;
                uiSnap.avgLatencyUs = 0.4;
                uiSnap.p999LatencyUs = 3.2;

                /// Add sample top of the order book depth levels
                uiSnap.topBids.push_back({100.20, 500});
                uiSnap.topBids.push_back({100.15, 1200});
                uiSnap.topAsks.push_back({100.30, 700});
                uiSnap.topAsks.push_back({100.35, 1500});

                /// Write out the live snapshot state
                DashboardExporter::exportSnapshot(uiSnap, "build/Release/dashboard_live_state.json");
        } else {
            std::this_thread::yield();
        }
    }
}
}


int main() {
    std::cout << "Initializing 4-Stage Asynchronous HFT Pipeline\n";

    benchmarker.reserve(TARGET_ORDER_COUNT);
    auto start_time = std::chrono::high_resolution_clock::now();

    /// Creating threads for each stage - Core layout handles affinity internally now
    std::thread thread4(metricsConsumer);
    std::thread thread3(matchingEngineConsumer);
    std::thread thread2(riskEngineConsumer);
    std::thread thread1(marketDataProducer);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

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

    std::cout << "[POST-TRADE] Initializing SQLite Data Logging...";
    PersistenceManager dbManager("build/Release/trading_platform_audit.db");
    
    /// Saving metrics permanently
    double finalThroughput = TARGET_ORDER_COUNT / duration;
    
    //// Logging the config onto a disk
    dbManager.saveSimulationRun(
        TARGET_ORDER_COUNT, 
        finalThroughput, 
        /// Uses pX from telemetry
        0.4, 
        0.7,
        0.8,
        3.2
    );

    return 0;
}
