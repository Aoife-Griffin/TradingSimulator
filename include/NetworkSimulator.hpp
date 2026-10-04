#ifndef NETWORK_SIMULATOR_HPP
#define NETWORK_SIMULATOR_HPP
#pragma once

#include <chrono>
#include <random>
#include <thread>

#include "Order.hpp"

namespace Trading {

/// Note: used
/// https://github.com/SteveMwika/Network-Simulation-Model-Exploring-Packet-Transmission/blob/main/proj2_Mwika_Steve.cpp
/// to understand this

class NetworkSimulator {
private:
    std::mt19937 m_rng{1337};
    std::uniform_real_distribution<double> m_probDist{0.0, 1.0};

    /// Network properties
    double m_packetLossRate{
        0.001};  /// https://github.com/SteveMwika/Network-Simulation-Model-Exploring-Packet-Transmission/blob/main/proj2_Mwika_Steve.cpp
                 /// for help deciding 0.001
    uint32_t m_baseLatencyNanos{15'000};
    uint32_t m_jitterNanos{5'000};

public:
    NetworkSimulator() = default;

    /// Returns true if the packet successfully cleared the network, false if lost
    bool transmitOrder(Order& order) {
        if (m_probDist(m_rng) < m_packetLossRate) {
            return false;  /// Packet was dropped
        }

        /// Simulate network and jitter
        int32_t currentJitter = static_cast<int32_t>((m_probDist(m_rng) * 2.0 - 1.0) * m_jitterNanos);
        uint64_t totalDelay = m_baseLatencyNanos + currentJitter;

        /// Delay the order so that it's processed immediately
        order.timestamp += totalDelay;

        return true;
    }
};

}  // namespace Trading
#endif
