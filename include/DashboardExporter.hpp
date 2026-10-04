#ifndef DASHBOARD_EXPORTER_HPP
#define DASHBOARD_EXPORTER_HPP
#pragma once

#include <fstream>
#include <iostream>

#include "DashboardTelemetry.hpp"

namespace Trading {

class DashboardExporter {
public:
    static void exportSnapshot(const DashboardSnapshot& snap, const std::string& outputPath) {
        std::ofstream file(outputPath);
        if (!file.is_open()) return;

        /// Output as JSON
        file << "  \"marketPrice\": " << snap.currentMarketPrice << ",\n";
        file << "  \"bestBid\": " << snap.bestBid << ",\n";
        file << "  \"bestAsk\": " << snap.bestAsk << ",\n";
        file << "  \"cashBalance\": " << snap.cashBalance << ",\n";
        file << "  \"netShares\": " << snap.netSharesOwned << ",\n";
        file << "  \"throughput\": " << static_cast<uint64_t>(snap.throughputOpsSec) << ",\n";
        file << "  \"p50LatencyUs\": " << snap.avgLatencyUs << ",\n";
        file << "  \"p999LatencyUs\": " << snap.p999LatencyUs << ",\n";

        /// Export Bids
        file << "  \"bids\": [\n";
        for (size_t i = 0; i < snap.topBids.size(); ++i) {
            file << "    {\"price\": " << snap.topBids[i].price << ", \"volume\": " << snap.topBids[i].volume << "}";
            if (i < snap.topBids.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ],\n";

        /// Export Asks
        file << "  \"asks\": [\n";
        for (size_t i = 0; i < snap.topAsks.size(); ++i) {
            file << "    {\"price\": " << snap.topAsks[i].price << ", \"volume\": " << snap.topAsks[i].volume << "}";
            if (i < snap.topAsks.size() - 1) file << ",";
            file << "\n";
        }
        file << "  ]\n";

        file << "}\n";
        file.close();
    }
};

}  // namespace Trading
#endif
