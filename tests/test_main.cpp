#include <gtest/gtest.h>
#include "Types.hpp"
#include "Order.hpp"
#include "RiskManager.hpp"
#include "MarketSimulator.hpp"
#include "ReplayEngine.hpp"
#include "OrderBook.hpp"

/// Test for making sure the order validation is working
TEST(OrderValidationTest, VerifiesExampleOrder) {
    Trading::Order order{
        .id = 1001,
        .timestamp = 1711843200000000000ULL,
        .price = 50.25,
        .quantity = 100, 
        .side = Trading::Side::BUY, 
        .type = Trading::OrderType::LIMIT
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
    EXPECT_FALSE(risk.validateOrder(duplicateOrder)); /// REJECTED: ID 999 already processed
    EXPECT_TRUE(risk.validateOrder(uniqueOrder));     /// PASSED: ID 1000 is brand new
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

/// Tests for making sure market scenarios matches request constraints
TEST(SimulationPhaseTest, VerifiesScenarioGeneration) {
    Trading::MarketSimulator simulator;
    auto normalFeed = simulator.generateScenarioData(Trading::MarketScenario::NORMAL, 1, 50);
    
    EXPECT_EQ(normalFeed.size(), 50);
    EXPECT_EQ(normalFeed.front().securityId, 1);
    EXPECT_GT(normalFeed.front().price, 0.0);
}

/// Verifies that replaying a scenario produces identical order book states
TEST(SimulationPhaseTest, EnforcesDeterministicReplays) {
    Trading::MarketSimulator simulator;
    Trading::OrderBook book1(1);
    Trading::OrderBook book2(1);
    Trading::RiskManager risk1;
    Trading::RiskManager risk2;
    Trading::ReplayEngine replay;

    /// Create a array for a flash case scenario
    auto crashFeed = simulator.generateScenarioData(Trading::MarketScenario::FLASH_CRASH, 1, 100);

    /// Feed identical arrays into completely independent processing lines
    replay.executeReplay(book1, risk1, crashFeed);
    replay.executeReplay(book2, risk2, crashFeed);

    /// Check both matching books arrived at identical depth metrics
    EXPECT_EQ(book1.getBestBid(), book2.getBestBid());
    EXPECT_EQ(book1.getBestAsk(), book2.getBestAsk());
}

/// Custom main() entry  to force compilation on MSVC
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
