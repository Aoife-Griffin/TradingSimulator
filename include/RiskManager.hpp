#pragma once
#include "Order.hpp"
#include <unordered_set>

namespace Trading {

    class RiskManager {
    private:
        std::unordered_set<OrderId> m_trackedOrderIds;

    public:
        bool validateOrder(const Order& order) {
            if (!order.isValid()) {
                return false;
            }
            if (order.type != OrderType::LIMIT && order.type != OrderType::MARKET) {
                return false;
            }

            /// Makes sure order ids are unique, rejecting duplicates
            if (m_trackedOrderIds.contains(order.id)) {
                return false; 
            }

            m_trackedOrderIds.insert(order.id);
            return true;
        }

        void clearRegistry() {
            m_trackedOrderIds.clear();
        }
    };
}
