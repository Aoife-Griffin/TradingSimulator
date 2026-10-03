#ifndef RISK_MANAGER_HPP
#define RISK_MANAGER_HPP

#pragma once
#include "Order.hpp"
#include "Types.hpp"
#include <unordered_set>
#include <string>
#include <cmath>

namespace Trading {

    struct RiskResult {
        enum Value : uint8_t {
            ACCEPTED,
            REJECTED_INVALID_ORDER_PRIMITIVES,
            REJECTED_DUPLICATE_ID,
            REJECTED_MAX_SIZE,
            REJECTED_MAX_POSITION,
            REJECTED_MAX_EXPOSURE,
            REJECTED_DAILY_LOSS_LIMIT,
            REJECTED_RATE_LIMIT
        };

        Value val;
        
        RiskResult(Value v) : val(v) {}
        operator bool() const { return val == ACCEPTED; }

        bool operator==(const RiskResult& other) const { return val == other.val; }
        bool operator==(Value otherVal) const { return val == otherVal; }
    };

    /// Convert RiskResult to string view. String view for removing copies and heap allocations
    inline std::string_view toString(RiskResult result) {
        switch(result.val) {
            case RiskResult::ACCEPTED: 
                return "ACCEPTED";
            case RiskResult::REJECTED_INVALID_ORDER_PRIMITIVES: 
                return "REJECTED: Invalid order primitives";
            case RiskResult::REJECTED_DUPLICATE_ID: 
                return "REJECTED: Duplicate ID";
            case RiskResult::REJECTED_MAX_SIZE: 
                return "REJECTED: Order size exceeds maximum size";
            case RiskResult::REJECTED_MAX_POSITION: 
                return "REJECTED: Order target violates asset share inventory limits";
            case RiskResult::REJECTED_MAX_EXPOSURE: 
                return "REJECTED: Capital exposure constraint hit";
            case RiskResult::REJECTED_DAILY_LOSS_LIMIT: 
                return "REJECTED: Daily drawdown boundary breached";
            case RiskResult::REJECTED_RATE_LIMIT: 
                return "REJECTED: Message high frequency speed rate limit triggered";
        }
        return "UNKNOWN";
    }


    /// Risk limits for the trading engine
    struct RiskLimits {
        uint32_t maxOrderSize{50'000};
        int32_t maxPositionSize{100'000};
        double maxNotionalExposure{5'000'000.0};
        double maxDailyLoss{50'000.0};
        uint32_t maxOrdersPerSecond{1000};
    };

    class RiskManager {
    private:
        std::unordered_set<OrderId> m_trackedOrderIds;
        RiskLimits m_limits;

    public:
        RiskResult validateOrder(const Order& order, int32_t currentAssetPosition = 0) {
            /// Basic structural sanity filter checks
            if (order.price <= 0 || order.quantity <= 0) {
                return RiskResult::REJECTED_INVALID_ORDER_PRIMITIVES;
            }

            /// Duplicate order id checks
            if (m_trackedOrderIds.contains(order.id)) {
                return RiskResult::REJECTED_DUPLICATE_ID;
            }

            /// Max Order Size filter
            if (order.quantity > m_limits.maxOrderSize) {
                return RiskResult::REJECTED_MAX_SIZE;
            }

            /// Inventory position checks to make sure don't exceed max position size
            int32_t projectedPosition = currentAssetPosition + 
                ((order.side == Side::BUY) ? static_cast<int32_t>(order.quantity) : -static_cast<int32_t>(order.quantity));
            
            if (std::abs(projectedPosition) > m_limits.maxPositionSize) {
                return RiskResult::REJECTED_MAX_POSITION;
            }

            /// Check to make sure we don't exceed our notional exposure limit
             double orderNotionalValue = order.quantity * order.price;
            if (orderNotionalValue > m_limits.maxNotionalExposure) {
                return RiskResult::REJECTED_MAX_EXPOSURE;
            }
            /// Track the order ID for uniqueness
            m_trackedOrderIds.insert(order.id);
            return RiskResult::ACCEPTED;
            } 


        void clearRegistry() {
            m_trackedOrderIds.clear();
        }
    }; 
} 

#endif 
