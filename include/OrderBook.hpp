#pragma once
#include "Order.hpp"
#include <vector>

namespace Trading {

    /// A price level for all orders at the price
    struct PriceLevel {
        Price price;
        Qty totalVolume;
        std::vector<Order> orders; /// REPLACE LATER WITH CUSTOM POOL ALLOCATOR (heap collisions)
    };

    /// A collection of price levels for a given security
    class OrderBook {
    private:
        SecurityId m_securityId;
        /// The best bid and ask should go to the top for easy access
        std::vector<PriceLevel> m_bids; 
        std::vector<PriceLevel> m_asks; 

    public:
        explicit OrderBook(SecurityId securityId) : m_securityId(securityId) {
            m_bids.reserve(1000); 
            m_asks.reserve(1000);
        }

        void insert(const Order& order);
        void cancel(OrderId id);
    };
}
