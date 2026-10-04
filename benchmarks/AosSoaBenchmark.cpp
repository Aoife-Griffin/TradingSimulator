#include <chrono>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using Price = double;
using Qty = uint32_t;
using OrderId = uint64_t;

constexpr size_t TEST_SIZE = 5'000'000;  // 5 Million orders for deep cache saturation

///     ARRAY OF STRUCTURES
struct AoS_Order {
    OrderId id;
    uint64_t timestamp;
    Price price;
    Qty quantity;
    uint16_t securityId;
    uint8_t side;
    uint8_t type;
};

///    STRUCTURE OF ARRAYS
struct SoA_OrderBook {
    std::vector<OrderId> ids;
    std::vector<uint64_t> timestamps;
    std::vector<Price> prices;
    std::vector<Qty> quantities;
    std::vector<uint16_t> securityIds;
    std::vector<uint8_t> sides;
    std::vector<uint8_t> types;

    /// Resize all vectors to the same size
    void resize(size_t size) {
        ids.resize(size);
        timestamps.resize(size);
        prices.resize(size);
        quantities.resize(size);
        securityIds.resize(size);
        sides.resize(size);
        types.resize(size);
    }
};

int main() {
    std::cout << "Element processing matrix scale: " << TEST_SIZE << " items\n\n";

    /// Initialize AoS Data
    std::vector<AoS_Order> aos_book(TEST_SIZE);
    for (size_t i = 0; i < TEST_SIZE; ++i) {
        aos_book[i].price = 100.0 + (i % 10);
        aos_book[i].quantity = 10 + (i % 5);
    }

    /// Initialize SoA Data
    SoA_OrderBook soa_book;
    soa_book.resize(TEST_SIZE);
    for (size_t i = 0; i < TEST_SIZE; ++i) {
        soa_book.prices[i] = 100.0 + (i % 10);
        soa_book.quantities[i] = 10 + (i % 5);
    }

    /// Test 1: Benchmark AoS Market Value Scan
    auto start_aos = std::chrono::high_resolution_clock::now();
    double aos_total_value = 0.0;

    /// Causes cache misses to see the performance
    for (size_t i = 0; i < TEST_SIZE; ++i) {
        aos_total_value += aos_book[i].price * aos_book[i].quantity;
    }

    /// Gets the end time and calculates the elapsed time in microseconds
    auto end_aos = std::chrono::high_resolution_clock::now();
    auto elapsed_aos = std::chrono::duration_cast<std::chrono::microseconds>(end_aos - start_aos).count();

    /// Test 2: Benchmark SoA Market Value Scan
    auto start_soa = std::chrono::high_resolution_clock::now();
    double soa_total_value = 0.0;

    /// Causes cache misses to see the performance
    for (size_t i = 0; i < TEST_SIZE; ++i) {
        soa_total_value += soa_book.prices[i] * soa_book.quantities[i];
    }

    auto end_soa = std::chrono::high_resolution_clock::now();
    auto elapsed_soa = std::chrono::duration_cast<std::chrono::microseconds>(end_soa - start_soa).count();

    /// Prints out results for comparison
    std::cout << "AoS Execution Time: " << elapsed_aos << " us\n";
    std::cout << "SoA Execution Time: " << elapsed_soa << " us\n";

    /// Calculate speedup factor for SoA over AoS
    double speedup = static_cast<double>(elapsed_aos) / elapsed_soa;
    std::cout << "Hardware Performance Delta: SoA is " << speedup << "x faster than AoS\n";

    /// Check that both methods produced the same total value to ensure correctness
    if (aos_total_value != soa_total_value) {
        std::cout << "Data verification mismatch anomaly warning.\n";
    }

    return 0;
}
