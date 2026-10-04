#pragma once
#include <cmath>
#include <unordered_map>

#include "Order.hpp"
#include "Types.hpp"

namespace Trading {

struct Position {
    int32_t shares{0};
    double averageEntryPrice{0.0};
    double realizedPnL{0.0};
    double unrealizedPnL{0.0};
};

class Portfolio {
private:
    double m_cash;
    double m_initialCash;
    std::unordered_map<SecurityId, Position> m_positions;

public:
    /// Portfolio should start with a certain amount to trade with
    explicit Portfolio(double startingCash) : m_cash(startingCash), m_initialCash(startingCash) {
    }

    /// Update portfolio when trade happens
    void updateOnTrade(const Trade& trade, Side executionSide) {
        auto& pos = m_positions[trade.securityId];

        if (executionSide ==
            Side::BUY) {  /// The buying side of the transaction, with money leaving as things are bought
            /// cash outflow = price * quantity
            m_cash -= (trade.executionPrice * trade.executionQuantity);

            /// Update position based on if it's a long or short position
            if (pos.shares >= 0) {
                double totalCost =
                    (pos.shares * pos.averageEntryPrice) + (trade.executionPrice * trade.executionQuantity);
                pos.shares += trade.executionQuantity;
                pos.averageEntryPrice = totalCost / pos.shares;
            } else {  /// If it's a short position, closing it out
                int32_t closedQty = std::min(static_cast<int32_t>(trade.executionQuantity), std::abs(pos.shares));
                pos.realizedPnL += (pos.averageEntryPrice - trade.executionPrice) * closedQty;
                pos.shares += trade.executionQuantity;

                /// If the position is now long, update the average entry price; if it's flat, reset it
                if (pos.shares > 0) {
                    pos.averageEntryPrice = trade.executionPrice;
                } else if (pos.shares == 0) {
                    pos.averageEntryPrice = 0.0;
                }
            }
        } else {  /// The selling side of the transaction, with money coming in as things are sold
            m_cash += (trade.executionPrice * trade.executionQuantity);

            if (pos.shares <= 0) {  /// Increasing Short Position
                double totalCost =
                    (std::abs(pos.shares) * pos.averageEntryPrice) + (trade.executionPrice * trade.executionQuantity);
                pos.shares -= trade.executionQuantity;
                pos.averageEntryPrice = totalCost / std::abs(pos.shares);
            } else {  // Closing out a Long Position
                int32_t closedQty = std::min(static_cast<int32_t>(trade.executionQuantity), pos.shares);
                pos.realizedPnL += (trade.executionPrice - pos.averageEntryPrice) * closedQty;
                pos.shares -= trade.executionQuantity;

                if (pos.shares < 0) {  /// Switched from long to short
                    pos.averageEntryPrice = trade.executionPrice;
                } else if (pos.shares == 0) {
                    pos.averageEntryPrice = 0.0;
                }
            }
        }
    }

    /// Update unrealized PnL based on current market price
    void updateUnrealizedPnL(SecurityId securityId, double currentMarketPrice) {
        auto it = m_positions.find(securityId);
        if (it != m_positions.end()) {
            auto& pos = it->second;
            if (pos.shares == 0) {
                pos.unrealizedPnL = 0.0;
            } else {
                pos.unrealizedPnL = (currentMarketPrice - pos.averageEntryPrice) * pos.shares;
            }
        }
    }

    double getTotalEquity() const {
        double totalPnL = 0.0;
        for (const auto& [id, pos] : m_positions) {
            totalPnL += pos.realizedPnL + pos.unrealizedPnL;
        }
        return m_initialCash + totalPnL;
    }

    double getCash() const {
        return m_cash;
    }
    const Position& getPosition(SecurityId id) {
        return m_positions[id];
    }
};
}  // namespace Trading
