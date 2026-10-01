#include <gtest/gtest.h>
#include "Types.hpp"
#include "Order.hpp"
#include "RiskManager.hpp"

/// Test for making sure the order validation is working
TEST(OrderValidationTest, VerifiesExampleOrder) {
    Trading::Order order{
        .id = 1001,
        .timestamp = 1711843200000000000ULL,
        .price = 50.25, // €50.25
        .quantity = 100, // 100 shares
        .side = Trading::Side::BUY, // BUY
        .type = Trading::OrderType::LIMIT // LIMIT
    };

    Trading::RiskManager risk;
    EXPECT_TRUE(risk.validateOrder(order));
    EXPECT_EQ(order.id, 1001);
    EXPECT_DOUBLE_EQ(order.price, 50.25);
    EXPECT_EQ(order.quantity, 100);
}

/// Testing for rejection of orders with 0 or negatives
TEST(OrderValidationTest, RejectsZeroOrNegativeQuantity) {
    Trading::Order invalidQtyOrder{
        .id = 1002,
        .timestamp = 100,
        .price = 50.25,
        .quantity = 0, 
        .side = Trading::Side::BUY,
        .type = Trading::OrderType::LIMIT
    };

    Trading::RiskManager risk;
    EXPECT_FALSE(risk.validateOrder(invalidQtyOrder));
}

/// Checks limit and market orders against invalid prices
TEST(OrderValidationTest, RejectsInvalidLimitPrices) {
    Trading::Order negativePriceOrder{
        .id = 1003,
        .timestamp = 100,
        .price = -5.00, 
        .quantity = 10,
        .side = Trading::Side::SELL,
        .type = Trading::OrderType::LIMIT
    };

    Trading::Order zeroPriceLimitOrder{
        .id = 1004,
        .timestamp = 100,
        .price = 0.0, 
        .quantity = 10,
        .side = Trading::Side::BUY,
        .type = Trading::OrderType::LIMIT
    };

    Trading::RiskManager risk;
    EXPECT_FALSE(risk.validateOrder(negativePriceOrder));
    EXPECT_FALSE(risk.validateOrder(zeroPriceLimitOrder));
}

/// Makes sure there are unique ids and rejects duplicates
TEST(OrderValidationTest, EnforcesUniqueOrderIDs) {
    Trading::RiskManager risk;

    Trading::Order firstOrder{.id = 999, .timestamp = 1, .price = 10.0, .quantity = 5, .side = Trading::Side::BUY, .type = Trading::OrderType::LIMIT};
    Trading::Order duplicateOrder{.id = 999, .timestamp = 2, .price = 11.0, .quantity = 5, .side = Trading::Side::BUY, .type = Trading::OrderType::LIMIT};
    Trading::Order uniqueOrder{.id = 1000, .timestamp = 3, .price = 12.0, .quantity = 5, .side = Trading::Side::BUY, .type = Trading::OrderType::LIMIT};

    EXPECT_TRUE(risk.validateOrder(firstOrder)); 
    EXPECT_FALSE(risk.validateOrder(duplicateOrder)); // REJECTED: ID 999 already processed
    EXPECT_TRUE(risk.validateOrder(uniqueOrder));     // PASSED: ID 1000 is brand new
}

/// Checks that trades have the correct primitives and are valid
TEST(TradeValidationTest, ValidatesTradePrimitives) {
    Trading::Trade trade{
        .tradeId = 1,
        .buyOrderId = 1001,
        .sellOrderId = 1002,
        .timestamp = 12345,
        .executionPrice = 50.25,
        .executionQuantity = 100
    };

    EXPECT_TRUE(trade.isValid());
}
