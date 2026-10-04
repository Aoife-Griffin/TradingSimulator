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

## 2. Live Simulator Controls
The system replicates realistic structural market dynamics:
1. **Normal & Trend Scenarios**: Models geometric Brownian random price walks with drift trends (Bull/Bear biases).
2. **Volatility Shocks & Flash Crashes**: Applies Poisson volume spikes to strip market depth layers and have protective risk rejections.



# Phase 4: Concurrecy & Performance Engineering
Optimized the architecture into a 4-stage asynchronous trading pipeline, using an atomic lock-free ring buffer aligned to cache line boundaries (`alignas(64)`) to reduce false sharing. 

## Production Performance Telemetry (20,000,000 Orders)
* **Throughput:** ~929,637 orders/second
* **Average Engine Latency:** 148.9 nanoseconds
* **p50 Latency:** 100.0 nanoseconds
* **p95 Latency:** 100.0 nanoseconds
* **p99 Latency:** 200.0 nanoseconds
* **p99.9 Tail Latency:** 3.9 microseconds

# Phase 5: Low-Level Code Optimization (Complete)

## 1. Fixing Code Bottlenecks
Instead of guessing how to make the code faster, Phase 5 focused on fixing real performance slowdowns found during testing:
* **Faster Text Handling (Day 17):** The risk checking system used to create slow text copies using standard strings (`std::string`). We changed this to use `std::string_view`. This lets the program read text directly from memory without making slow copies.
* **Pre-Allocated Memory (Day 18):** Creating and deleting orders on the fly makes the computer ask the Operating System for memory, which is very slow. We built a custom **Memory Pool** (`OrderPool`). Now, memory for 200,000 orders is set aside before the system starts, making order creation incredibly fast.
* **Locking Threads to CPU Cores (Day 19):** By default, Windows moves tasks between different CPU cores. This slows things down because the CPU has to keep reload data. We used Windows commands to lock each of our 4 pipeline steps onto its own permanent CPU core. This keeps the data exactly where the CPU can reach it fastest.


## 2. Memory Layout Experiment: Array of Structures (AoS) vs. Structure of Arrays (SoA)
To see how memory layout changes speed, we wrote a test program (`Benchmarks/AosSoaBenchmark.cpp`). It calculates the total financial value of **5,000,000 orders** using two different approaches.

### Test Results:
* **Approach 1 - AoS (Mixed Data Layout):** `21,538 us`
* **Approach 2 - SoA (Separated Data Layout):** `9,630 us`
* **Performance Difference:** **The SoA layout ran 2.24x faster than AoS.**


# Phase 6: Networking, Strategies & Persistence (Complete)
## 1. Multi-Component Platform Infrastructure
Turned the standalone matching engine into a realistic system that models network lag, different trader behaviors, and system stress:
* **Network Wire Simulation:** Adds realistic lag and random delays to mimic web traffic before orders hit the engine. It also drops occasional packets to simulate internet connectivity issues.
* **Trading Strategy Simulation:** Runs 1,000 automated trading bots at the same time. These include Market Makers (setting tight buy/sell prices), Momentum Traders (following price trends), and Random Traders (adding market noise).
* **Chaos Engineering & Failure Testing:** Intentionally injects bad data into the system, including extreme high-volume surges, "fat-finger" errors (negative prices), and massive order sizes. The system processes these spikes safely without crashing.
* **Database Persistence Subsystem:** Saves simulation runs, order history, and account metrics to a permanent database file (`trading_platform_audit.db`). To keep the system running at maximum speed, this saving process happens only *after* all trading ends.

## 2. High-Stress Performance Profile (1,000 Traders | 20,000,000 Orders)
* **Ecosystem Throughput:** 1,072,830 orders/second (Maintained over 1M ops/sec with active chaos injecting)
* **Average Latency:** 0.49 microseconds
* **p50 Latency:** 0.40 microseconds
* **p95 Latency:** 0.80 microseconds
* **p99 Latency:** 1.10 microseconds
* **p99.9 Tail Latency:** 4.20 microseconds


# Phase 7: Dashboard, Testing & CI/CD Automation (Complete)

Turned the project into a fully visible, production-tested, and automatically deployed enterprise-grade trading ecosystem.

## 1. Frontend Workstation Dashboard
* Built a modern, dark-themed user interface using **React and TypeScript** powered by Vite, inspired by tradingview's pages (https://www.tradingview.com/trading/)
* Polled an asynchronous telemetry snapshot output stream cleanly out of the critical C++ processing paths to prevent engine stalling.
* Uses **oxlint (Rust-based static linter)** to enforce high-performance frontend code quality without configuration clutter.
* Visualizes real-time order book green/red depth ladders, active inventory share counts, running cash positions, throughput processing speeds (`ops/sec`), and sub-microsecond tail latency counters.

## 2. Automated Quality Assurance
* Expanded the automated **GoogleTest** suite to enforce critical execution invariants across memory management vectors.
* Validates that the custom `OrderPool` Free List cleanly recycles elements in constant O(1) space, maintaining flat memory lines under extreme strain.
* Verifies structural boundary shields across the `RiskManager` to block fat-finger execution mistakes and volume-breach limits safely.

## 3. Continuous Integration Pipeline (DevOps)
* Configured an automated **GitHub Actions Workflow** that triggers on every code check-in.
* Enforces strict code formatting rules across all files via **`clang-format`**.
* Automatically provisions Ubuntu cloud servers to build the core libraries, execute GoogleTest verification layers, and run memory-subsystem cache regressions (`aos_soa_bench`) to guarantee new commits never degrade our **1.1M orders/sec processing baseline**.
