# High-Performance Trading Simulator

A low-latency trading simulation engine built with C++20.


# Phase 1: Foundation
## 1. Project Setup
* Project layout established.
* CMake structural build setup complete.
* GoogleTest integration verified.

## Compilation and Testing Instructions
```bash
cmake -B build
cmake --build build
cd build && ctest --output-on-failure
```
## 2. System Design
* Architecture designed with diagram used
* Used data oriented design, which works well for L1 and L2 caches

**Core Objects Overview**
*   Order: A user's request to buy or sell an asset at a specific price and size.
*   Trade: A permanent record created whenever a buyer and a seller match in the book.
*   PriceLevel: A group of active orders sitting at the exact same price point.
*   OrderBook: A sorted list of active buy requests (Bids) and sell requests (Asks).
*   MatchingEngine: The core brain that checks risk, looks for matches, and executes trades.
*   Portfolio: A tracker that displays how many shares you own and your current trading profits.
*   RiskManager: A fast security gate that blocks invalid prices, huge sizes, or rogue orders.
*   MarketData: A continuous stream of live prices fed directly into the engine memory.

## 3. Core Trading Models
**Core Layout Footprints**
*   Order Struct (32 Bytes):  Arranged from largest to smallest data type to eliminate hidden padding bytes.
*   Trade Struct (48 Bytes): • Includes a nanosecond timestamp matching up to an 8 byte boundary.


**Engine Validation Rules**
Every order must pass these four filters:
1.  Quantity Validation: Must be strictly greater than 0.
2.  Price Boundaries: Limit orders must be above 0.0; market orders accept 0.0.
3.  Unique Identifier Checks: The `RiskManager` blocks any incoming order with a duplicate ID.
4.  Strong Typing Enforcement: Enforced by strong types to ensure it is only BUY/SELL and LIMIT/MARKET.


# Phase 2: Order Book & Matching Engine

# Phase 3: Portfolio, Risk & Market Simulation

# Phase 4: Concurrecy & Performance Engineering