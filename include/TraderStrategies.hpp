#ifndef TRADER_STRATEGIES_HPP
#define TRADER_STRATEGIES_HPP
#pragma once

#include <random>

#include "Order.hpp"

namespace Trading {

enum class TraderType : uint8_t { MARKET_MAKER, MOMENTUM, RANDOM };

class Trader {
private:
    uint32_t m_id;
    TraderType m_type;
    std::mt19937 m_rng;

public:
    Trader(uint32_t id, TraderType type) : m_id(id), m_type(type), m_rng(id) {
    }

    /// Generates the next order using the trader's strategy and the  market price
    Order generateNextOrder(Price currentMarketPrice, OrderId uniqueId) {
        Order order;
        order.id = uniqueId;
        order.securityId = 1;
        order.type = OrderType::LIMIT;

        /// if the trader is a market maker, they will provide liquidity close to market price
        if (m_type == TraderType::MARKET_MAKER) {
            bool isBuy = (uniqueId % 2 == 0);
            order.side = isBuy ? Side::BUY : Side::SELL;
            order.price = isBuy ? (currentMarketPrice - 0.05) : (currentMarketPrice + 0.05);
            order.quantity = 100;
        }
        /// else if the trader is a momentum trader, they will follow the trend of the market
        else if (m_type == TraderType::MOMENTUM) {
            order.side = (currentMarketPrice > 100.0) ? Side::BUY : Side::SELL;
            order.price = currentMarketPrice;
            order.quantity = 200;
        }
        /// else if the trader is a random trader, they will place orders randomly
        else {
            order.side = (m_rng() % 2 == 0) ? Side::BUY : Side::SELL;
            order.price = currentMarketPrice + ((m_rng() % 20 - 10) * 0.1);
            order.quantity = ((m_rng() % 5) + 1) * 10;
        }
        return order;
    }
};

}  // namespace Trading
#endif
