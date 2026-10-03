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

## 1. Core Engine Design Principles
*   **Contiguous Memory**: Bids and Asks are stored in arrays pre-allocated to 1,000 slots to avoid slow memory reallocations during live trading.
*   **Price-Time Priority**: 
    *   **Price**: Uses fast binary search (`std::lower_bound`) to keep the best prices right at index 0.
    *   **Time**: Appends new orders to the back of their price group to guarantee a strict first-come, first-served queue.

## 2. How the Engine Works
1.  **Order Book Operations**: Manages incoming orders by adding, cancelling, or modifying resting limit queues (`addOrder`, `cancelOrder`, `modifyOrder`).
2.  **Matching Engine Processing**:  Scans the book to instantly fill incoming volumes against resting queues, handling partial fills and market order sweeps.
3.  **Trade Issuance**: Generates permanent, unique trade execution ticks the millisecond a match occurs.


# Phase 3: Portfolio, Risk & Market Simulation

## 1. Core Principals
* **Stateful Account Accounting**: Positions and live cash values are updated dynamically on a ledger whenever a match executes.
* **Complex Multi-Bound Risk Guarding**: Incoming commands are filtered by the `RiskManager` against maximum size bounds, capital exposure caps, and net position velocity counts.
* **Deterministic Environment Replays**: Sequential historical transaction files read  identical portfolio states due to a chronological clock tree.

### 2. Live Simulator Controls
The system replicates realistic structural market dynamics:
1. **Normal & Trend Scenarios**: Models geometric Brownian random price walks with drift trends (Bull/Bear biases).
2. **Volatility Shocks & Flash Crashes**: Applies Poisson volume spikes to strip market depth layers and have protective risk rejections.



# Phase 4: Concurrecy & Performance Engineering