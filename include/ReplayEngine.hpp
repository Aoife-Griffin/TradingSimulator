#pragma once
#include "OrderBook.hpp"
#include "RiskManager.hpp"
#include <vector>
#include <iostream>

namespace Trading {

    class ReplayEngine {
    public:
        /// History of orders can be replayed to test the trading engine
        void executeReplay(OrderBook& book, RiskManager& risk, const std::vector<Order>& historicFeed) {
            uint32_t processedCount = 0;
            uint32_t rejectedCount = 0;

            for (const auto& order : historicFeed) {
                /// Validate the order against risk checks before adding to the order book
                RiskResult check = risk.validateOrder(order, 0);

                if (check == RiskResult::ACCEPTED) {
                    /// Orders are added to the order book if they pass risk checks
                    book.addOrder(order);
                    processedCount++;
                } else {
                    rejectedCount++;
                }
            }

            std::cout << "[REPLAY ENGINE] Run complete. Orders Accepted: " 
                      << processedCount << " | Rejected: " << rejectedCount << "\n";
        }
    };
}
