#pragma once
#include "OrderBook.hpp"
#include <unordered_map>

namespace Trading {

    class RiskManager {
    public:
        /// Check if the order passes risk checks before sending to the order book
        bool checkRisk(const Order& order, Qty currentPosition) {
            if (order.quantity > 100'000) return false; // Fat-finger block
            if (currentPosition + order.quantity > 500'000) return false; // Position boundary check
            return true;
        }
    };

    /// Portfolio that tracks positions (net amount of assets) and PNL (profit & loss) for an account
    class Portfolio {
    private:
        std::unordered_map<SecurityId, int32_t> m_positions; 
        double m_realizedPnL{0.0};

    public:
    /// Position changed when trade happens
        void updateOnTrade(const Trade& trade, Side side) {
            int32_t qtyChange = (side == Side::BUY) ? trade.executionQuantity : -static_cast<int32_t>(trade.executionQuantity);
            m_positions[trade.securityId] += qtyChange;
        }
    };

    class MatchingEngine {
        /// Processes orders and matches them against order books while keeping an eye on risks and portfolio
    private:
        std::unordered_map<SecurityId, OrderBook> m_books;
        RiskManager m_riskManager;
        Portfolio m_portfolio;

    public:
        void processOrder(const Order& order) {
            
        }
    };
}
