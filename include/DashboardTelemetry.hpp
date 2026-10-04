#ifndef DASHBOARD_TELEMETRY_HPP
#define DASHBOARD_TELEMETRY_HPP
#pragma once

#include <string>
#include <vector>

#include "Order.hpp"

namespace Trading {

/// Depth blocks
struct UiLevel {
    double price;
    uint32_t volume;
};

struct DashboardSnapshot {
    double currentMarketPrice{100.0};
    double bestBid{0.0};
    double bestAsk{0.0};

    /// Accounts Ledger
    double cashBalance{1000000.0};
    double realizedPnL{0.0};
    int32_t netSharesOwned{0};

    /// Engine Speed Telemetry
    double throughputOpsSec{0.0};
    double avgLatencyUs{0.0};
    double p999LatencyUs{0.0};

    /// Sorts the depth of top of the book bids and asks
    std::vector<UiLevel> topBids;
    std::vector<UiLevel> topAsks;
};

}  // namespace Trading
#endif
