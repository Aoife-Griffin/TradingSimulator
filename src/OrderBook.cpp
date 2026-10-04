#include "OrderBook.hpp"
#include <algorithm>

namespace Trading {

/// Orderbook Constructor
OrderBook::OrderBook(SecurityId securityId) : m_securityId(securityId) {
    m_bids.reserve(1000);
    m_asks.reserve(1000);
}

/// Find or insert a price level for the book
std::vector<PriceLevel>::iterator OrderBook::findOrInsertLevel(std::vector<PriceLevel>& levels, Price price,
                                                              bool descending) {
    auto it = std::find_if(levels.begin(), levels.end(), [price](const PriceLevel& lvl) { return lvl.price == price; });

    if (it != levels.end()) return it;

    PriceLevel newLevel = {.price = price};
    auto insertPos = std::lower_bound(
        levels.begin(), levels.end(), newLevel,
        [descending](const PriceLevel& a, const PriceLevel& b) {
            return descending ? (a.price > b.price) : (a.price < b.price);
        });

    return levels.insert(insertPos, newLevel);
}

/// Add an order, pushing it to the back for time priority
void OrderBook::addOrder(const Order& order) {
    auto& levels = (order.side == Side::BUY) ? m_bids : m_asks;
    auto it = findOrInsertLevel(levels, order.price, order.side == Side::BUY);

    /// Changed to use OrderPool for memory management
    int poolIdx = m_pool.allocate(order);
    it->orderPoolIndices.push_back(poolIdx);
    it->totalVolume += order.quantity;

    it->totalVolume -= order.quantity;
    m_pool.deallocate(poolIdx);
    it->orderPoolIndices.pop_back();

    /// Cleaning iterator
    if (it->orderPoolIndices.empty()) {
        auto erasePos = std::find_if(levels.begin(), levels.end(),
                                     [order](const PriceLevel& lvl) { return lvl.price == order.price; });
        if (erasePos != levels.end()) {
            levels.erase(erasePos);
        }
    }
}

/// Cancel order
void OrderBook::cancelOrder(OrderId id, Side side) {
    auto& levels = (side == Side::BUY) ? m_bids : m_asks;
    for (auto lvlIt = levels.begin(); lvlIt != levels.end(); ++lvlIt) {
        auto& indexQueue = lvlIt->orderPoolIndices;

        auto matchIt = std::find_if(indexQueue.begin(), indexQueue.end(),
                                    [&](int poolIdx) { return m_pool.get(poolIdx).id == id; });

        /// Adjust volume if deleted and delete if empty
        if (matchIt != indexQueue.end()) {
            int poolIdxToFree = *matchIt;
            lvlIt->totalVolume -= m_pool.get(poolIdxToFree).quantity;

            /// Deallocate the order from the pool and remove it from the queue
            m_pool.deallocate(poolIdxToFree);
            indexQueue.erase(matchIt);

            if (indexQueue.empty()) {
                levels.erase(lvlIt);
            }
            return;
        }
    }
}

/// For high priority, orders can be canceled and readded with new parameters
void OrderBook::modifyOrder(OrderId id, Side side, Qty newQty, Price newPrice) {
    cancelOrder(id, side);
    Order modifiedOrder = {.id = id,
                           .timestamp = 0,
                           .price = newPrice,
                           .quantity = newQty,
                           .securityId = m_securityId,
                           .side = side,
                           .type = OrderType::LIMIT};
    addOrder(modifiedOrder);
}

std::optional<Price> OrderBook::getBestBid() const {
    return m_bids.empty() ? std::nullopt : std::make_optional(m_bids.front().price);
}

std::optional<Price> OrderBook::getBestAsk() const {
    return m_asks.empty() ? std::nullopt : std::make_optional(m_asks.front().price);
}

}  // namespace Trading
