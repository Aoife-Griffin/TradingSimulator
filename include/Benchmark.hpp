#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP
#pragma once

#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>

namespace Trading {

struct TimingPoint {
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
};

class PerformanceBenchmarker {
private:
    std::vector<uint64_t> m_latenciesNanos;

public:
    /// Reserve space for latency measurement
    void reserve(size_t capacity) {
        m_latenciesNanos.reserve(capacity);
    }

    /// Record the latency of a single operation in nanoseconds
    void recordLatency(std::chrono::high_resolution_clock::time_point start, 
                       std::chrono::high_resolution_clock::time_point end) {
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        m_latenciesNanos.push_back(duration);
    }

    /// Pull percentiles from sorted vectors
    uint64_t getPercentile(double pct) const {
        if (m_latenciesNanos.empty()) return 0;
        size_t index = static_cast<size_t>(pct * (m_latenciesNanos.size() - 1));
        return m_latenciesNanos[index];
    }

    void generateReport(double totalDurationSeconds, size_t totalOrders) {
        if (m_latenciesNanos.empty()) {
            std::cout << "No performance metrics captured.";
            return;
        }

        /// Get percentile thresholds from sorted latencies
        std::sort(m_latenciesNanos.begin(), m_latenciesNanos.end());

        double ops = totalOrders / totalDurationSeconds;
        uint64_t sum = std::accumulate(m_latenciesNanos.begin(), m_latenciesNanos.end(), 0ULL);
        double avg = static_cast<double>(sum) / m_latenciesNanos.size();

        
        std::cout << "Throughput:      " << ops << " orders/sec\n";
        std::cout << "Average Latency: " << avg / 1000.0 << " us\n";
        std::cout << "p50 Latency:     " << getPercentile(0.50) / 1000.0 << " us\n";
        std::cout << "p95 Latency:     " << getPercentile(0.95) / 1000.0 << " us\n";
        std::cout << "p99 Latency:     " << getPercentile(0.99) / 1000.0 << " us\n";
        std::cout << "p99.9 Latency:   " << getPercentile(0.999) / 1000.0 << " us\n";
    }
};

} 
#endif
