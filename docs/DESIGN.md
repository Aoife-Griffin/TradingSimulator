# Software Design & Data Structure Profiling

## 1. Pre-Allocated Memory Pool (`OrderPool`)
* **The Problem:** Creating and deleting orders on the fly makes the computer ask the Operating System for memory, which introduces unpredictable lag spikes and memory clutter on the active trading path.
* **The Solution:** We built a custom **Memory Pool** (`OrderPool`) that sets aside space for 1,000,000 orders when the system initializes.
* **How It Works:** Free slots are linked together in a simple list using integer indices. Fetching an empty slot or returning an old order takes a constant **O(1) time** via simple pointer updates. This completely keeps the slow OS memory manager out of our live matching loops.

## 2. Smart Memory Packing (Spatial Locality)
The variables inside the `Order` structure are manually arranged from largest data type size down to the smallest. This eliminates hidden padding spaces that compilers insert automatically:
* `OrderId id` (8 Bytes)
* `uint64_t timestamp` (8 Bytes)
* `Price price` (8 Bytes)
* `Qty quantity` (4 Bytes)
* `SecurityId securityId` (2 Bytes)
* `Side side` / `OrderType type` (2 Bytes combined)
* **Total Size:** Exactly **32 Bytes**. This allows two full orders to fit perfectly inside a single **64-byte CPU cache line**, packing the computer's memory tightly for maximum speed.
