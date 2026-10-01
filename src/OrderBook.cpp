#include "OrderBook.hpp"

namespace Trading {

    /// Orderbook Constructor
    OrderBook::OrderBook(SecurityId securityId) : m_securityId(securityId) {
        m_bids.reserve(256); // Pre-allocate memory lines to shield against heap thrashing
        m_asks.reserve(256);
    }

    /// Find or insert a price level for the book
    std::vector<PriceLevel>::iterator OrderBook::findOrInsertLevel(std::vector<PriceLevel>& levels, Price price, bool descending) {
        auto it = std::find_if(levels.begin(), levels.end(), [price](const PriceLevel& lvl) {
            return lvl.price == price;
        });

        if (it != levels.end()) return it;

        PriceLevel newLevel{.price = price};
        auto insertPos = std::lower_bound(levels.begin(), levels.end(), newLevel, [descending](const PriceLevel& a, const PriceLevel& b) {
            return descending ? (a.price > b.price) : (a.price < b.price);
        });
        
        return levels.insert(insertPos, newLevel);
    }

    /// Add an order, pushing it to the back for time priority
    void OrderBook::addOrder(const Order& order) {
        auto& levels = (order.side == Side::BUY) ? m_bids : m_asks;
        auto it = findOrInsertLevel(levels, order.price, order.side == Side::BUY);
        it->orders.push_back(order); 
        it->totalVolume += order.quantity;
    }

    /// Cancel order
    void OrderBook::cancelOrder(OrderId id, Side side) {
        auto& levels = (side == Side::BUY) ? m_bids : m_asks;
        for (auto lvlIt = levels.begin(); lvlIt != levels.end(); ++lvlIt) {
            auto& queue = lvlIt->orders;
            auto matchIt = std::find_if(queue.begin(), queue.end(), [id](const Order& o) { return o.id == id; });
            
            /// Adjust volume if deleted and delete if empty
            if (matchIt != queue.end()) {
                lvlIt->totalVolume -= matchIt->quantity;
                queue.erase(matchIt);
                if (queue.empty()) {
                    levels.erase(lvlIt);
                }
                return;
            }
        }
    }

    /// For high priority, orders can be canceled and readded with new parameters
    void OrderBook::modifyOrder(OrderId id, Side side, Qty newQty, Price newPrice) {
        cancelOrder(id, side);
        Order modifiedOrder{.id = id, .timestamp = 0, .price = newPrice, .quantity = newQty, .securityId = m_securityId, .side = side, .type = OrderType::LIMIT};
        addOrder(modifiedOrder);
    }

    std::optional<Price> OrderBook::getBestBid() const {
        return m_bids.empty() ? std::nullopt : std::make_optional(m_bids.front().price);
    }

    std::optional<Price> OrderBook::getBestAsk() const {
        return m_asks.empty() ? std::nullopt : std::make_optional(m_asks.front().price);
    }
}
