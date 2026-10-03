#pragma once
#include "Order.hpp"
#include "OrderPool.hpp"
#include <vector>
#include <optional>
#include <algorithm>

namespace Trading {

    /// A price level for all orders at the price
    struct PriceLevel {
        Price price{0.0};
        Qty totalVolume{0};
        std::vector<int> orderPoolIndices; 
    };

    /// A collection of price levels for a given security
    class OrderBook {
    private:
        SecurityId m_securityId;
        /// The best bid and ask should go to the top for easy access
        std::vector<PriceLevel> m_bids; 
        std::vector<PriceLevel> m_asks; 

        /// Pre-allocated pool for orders to avoid dynamic memory allocation during trading
        OrderPool<200'000> m_pool; 

        /// Helper to find or insert a price level
        std::vector<PriceLevel>::iterator findOrInsertLevel(std::vector<PriceLevel>& levels, Price price, bool descending);



    public:
        explicit OrderBook(SecurityId securityId);

        /// Core API Features
        void addOrder(const Order& order);
        void cancelOrder(OrderId id, Side side);
        void modifyOrder(OrderId id, Side side, Qty newQty, Price newPrice);

        /// Accessors for best bid and ask
        std::optional<Price> getBestBid() const;
        std::optional<Price> getBestAsk() const;

        /// Accessors for the order book
        std::vector<PriceLevel>& getBids() { return m_bids; }
        std::vector<PriceLevel>& getAsks() { return m_asks; }
    };
}
