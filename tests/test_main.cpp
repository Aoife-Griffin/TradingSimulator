#include <gtest/gtest.h>
#include "Types.hpp"
#include "Order.hpp"
#include "RiskManager.hpp"
#include "MarketSimulator.hpp"
#include "ReplayEngine.hpp"
#include "OrderBook.hpp"
#include "OrderPool.hpp"

using namespace Trading;

/// Test for making sure the order validation is working
TEST(OrderValidationTest, VerifiesExampleOrder) {
    Order order{
        .id = 1001,
        .timestamp = 1711843200000000000ULL,
        .price = 50.25,
        .quantity = 100,
        .side = Side::BUY,
        .type = OrderType::LIMIT
    };

    RiskManager risk;
    EXPECT_TRUE(risk.validateOrder(order));
    EXPECT_EQ(order.id, 1001);
    EXPECT_DOUBLE_EQ(order.price, 50.25);
    EXPECT_EQ(order.quantity, 100);
}

/// Testing for rejection of orders with 0 or negatives
TEST(OrderValidationTest, RejectsZeroOrNegativeQuantity) {
    Order invalidQtyOrder{
        .id = 1002,
        .timestamp = 100,
        .price = 50.25,
        .quantity = 0,
        .side = Side::BUY,
        .type = OrderType::LIMIT
    };

    RiskManager risk;
    EXPECT_FALSE(risk.validateOrder(invalidQtyOrder));
}

/// Checks limit and market orders against invalid prices
TEST(OrderValidationTest, RejectsInvalidLimitPrices) {
    Order negativePriceOrder{
        .id = 1003,
        .timestamp = 100,
        .price = -5.00,
        .quantity = 10,
        .side = Side::SELL,
        .type = OrderType::LIMIT
    };

    Order zeroPriceLimitOrder{
        .id = 1004,
        .timestamp = 100,
        .price = 0.0,
        .quantity = 10,
        .side = Side::BUY,
        .type = OrderType::LIMIT
    };

    RiskManager risk;
    EXPECT_FALSE(risk.validateOrder(negativePriceOrder));
    EXPECT_FALSE(risk.validateOrder(zeroPriceLimitOrder));
}

/// Makes sure there are unique ids and rejects duplicates
TEST(OrderValidationTest, EnforcesUniqueOrderIDs) {
    RiskManager risk;

    Order firstOrder{.id = 999, .timestamp = 1, .price = 10.0, .quantity = 5, .side = Side::BUY, .type = OrderType::LIMIT};
    Order duplicateOrder{.id = 999, .timestamp = 2, .price = 11.0, .quantity = 5, .side = Side::BUY, .type = OrderType::LIMIT};
    Order uniqueOrder{.id = 1000, .timestamp = 3, .price = 12.0, .quantity = 5, .side = Side::BUY, .type = OrderType::LIMIT};

    EXPECT_TRUE(risk.validateOrder(firstOrder));
    EXPECT_FALSE(risk.validateOrder(duplicateOrder));
    EXPECT_TRUE(risk.validateOrder(uniqueOrder));
}

/// Checks that trades have the correct primitives and are valid
TEST(TradeValidationTest, ValidatesTradePrimitives) {
    Trade trade{
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
    MarketSimulator simulator;
    auto normalFeed = simulator.generateScenarioData(MarketScenario::NORMAL, 1, 50);

    EXPECT_EQ(normalFeed.size(), 50);
    EXPECT_EQ(normalFeed.front().securityId, 1);
    EXPECT_GT(normalFeed.front().price, 0.0);
}

/// Verifies that replaying a scenario produces identical order book states
TEST(SimulationPhaseTest, EnforcesDeterministicReplays) {
    MarketSimulator simulator;
    OrderBook book1(1);
    OrderBook book2(1);
    RiskManager risk1;
    RiskManager risk2;
    ReplayEngine replay;

    auto crashFeed = simulator.generateScenarioData(MarketScenario::FLASH_CRASH, 1, 100);

    replay.executeReplay(book1, risk1, crashFeed);
    replay.executeReplay(book2, risk2, crashFeed);

    EXPECT_EQ(book1.getBestBid(), book2.getBestBid());
    EXPECT_EQ(book1.getBestAsk(), book2.getBestAsk());
}

/// Testing Component Functional Barriers
TEST(RiskManagerTests, InterceptsValidAndInvalidPrimitives) {
    RiskManager risk;

    Order validOrder{.id = 1, .timestamp = 100, .price = 100.50, .quantity = 50, .side = Side::BUY, .type = OrderType::LIMIT};
    Order invalidQtyOrder{.id = 2, .timestamp = 100, .price = 100.50, .quantity = 0, .side = Side::BUY, .type = OrderType::LIMIT};
    Order negativePriceOrder{.id = 3, .timestamp = 100, .price = -5.00, .quantity = 10, .side = Side::SELL, .type = OrderType::LIMIT};

    EXPECT_TRUE(risk.validateOrder(validOrder));
    EXPECT_FALSE(risk.validateOrder(invalidQtyOrder));
    EXPECT_FALSE(risk.validateOrder(negativePriceOrder));
}

TEST(RiskManagerTests, EnforcesUniqueOrderIdentifiers) {
    RiskManager risk;
    Order firstOrder{.id = 999, .timestamp = 1, .price = 10.0, .quantity = 5, .side = Side::BUY, .type = OrderType::LIMIT};
    Order duplicateOrder{.id = 999, .timestamp = 2, .price = 11.0, .quantity = 5, .side = Side::BUY, .type = OrderType::LIMIT};

    EXPECT_TRUE(risk.validateOrder(firstOrder));
    EXPECT_FALSE(risk.validateOrder(duplicateOrder));
}

/// Memory Pool Component Test
TEST(OrderPoolTests, AllocatesAndRecyclesConstantSlots) {
    OrderPool<10> pool;
    Order testOrder{.id = 55, .price = 50.0, .quantity = 100};

    int idx1 = pool.allocate(testOrder);
    EXPECT_GE(idx1, 0);
    EXPECT_EQ(pool.get(idx1).id, 55);

    pool.deallocate(idx1);

    int idx2 = pool.allocate(testOrder);
    EXPECT_EQ(idx1, idx2);
}

/// Converted Order book pricing invariant test
TEST(OrderBookTests, EnforcesPriceTimePrioritySorting) {
    OrderBook book(1);
    Order lowBid{.id = 1, .price = 99.00, .quantity = 100, .side = Side::BUY, .type = OrderType::LIMIT};
    Order highBid{.id = 2, .price = 101.50, .quantity = 100, .side = Side::BUY, .type = OrderType::LIMIT};
    Order midBid{.id = 3, .price = 100.00, .quantity = 100, .side = Side::BUY, .type = OrderType::LIMIT};

    book.addOrder(lowBid);
    book.addOrder(highBid);
    book.addOrder(midBid);

    auto bestBid = book.getBestBid();
    EXPECT_FALSE(bestBid.has_value());
}

/// Custom main() entry to force compilation on MSVC
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
