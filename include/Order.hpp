#pragma once
#include "Types.hpp"

namespace Trading {

    /// The size of the order has to be 32 bytes so it can fit inside a single cache line
    /// Note: used https://www.geeksforgeeks.org/c/structure-member-alignment-padding-and-data-packing/ for help in understanding padding and alignment
    struct Order {
        OrderId id;
        uint64_t timestamp; 

        Price price;
        Qty quantity;
        SecurityId securityId;
        Side side;
        OrderType type;

        /// Checking if order is valid
         bool isValid() const {
            if (quantity == 0) return false;
            
            /// Limit orders must have a positive price
            if (type == OrderType::LIMIT && price <= 0.0) return false;
            
            /// Market orders ignore incoming pricing structures for immediate execution & checks that the price isn't negative
            if (type == OrderType::MARKET && price < 0.0) return false;
            
            return true;
        }
    };

    
    struct Trade {
        uint64_t tradeId;
        OrderId buyOrderId;
        OrderId sellOrderId;
        uint64_t timestamp;
        Price executionPrice;
        Qty executionQuantity;
        SecurityId securityId;
        uint8_t padding[2];  /// Padding to make sure the struct is 32 bytes
    
        bool isValid() const {
            return executionQuantity > 0 && executionPrice > 0.0 && buyOrderId != 0 && sellOrderId != 0;
        }
    };
}
