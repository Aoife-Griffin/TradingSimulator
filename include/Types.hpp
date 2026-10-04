#pragma once
#include <cstdint>

/// Primititve types for the trading engine
namespace Trading {
using SecurityId = uint16_t;
using OrderId = uint64_t;
using Price = double;
using Qty = uint32_t;

enum class Side : uint8_t { BUY, SELL };
enum class OrderType : uint8_t { LIMIT, MARKET };
enum class OrderStatus : uint8_t { NEW, FILLED, PARTIALLY_FILLED, CANCELLED, REJECTED };
}  // namespace Trading
