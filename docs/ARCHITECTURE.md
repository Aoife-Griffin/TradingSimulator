# System Architecture Specification

## 1. The 4 Stage Processing Pipeline
To keep things running at maximum speed, the system splits work across 4 separate steps. Each step runs on its own dedicated CPU core. This keeps data exactly where the CPU can reach it fastest and prevents the computer from slowing down.

## 2. Thread Coordination & Synchronization
* **Lock-Free Communication:** Threads pass data using a custom `LockFreeQueue`. It avoids slow operating system locks, using smart memory barriers to pass data instantly.
* **Stopping Core Interference:** Internal tracking pointers are separated by 64-byte boundaries (`alignas(64)`). This guarantees that Core 0 updating the queue never forces Core 1 to reload its active memory.
* **Fast Spinning Loops:** Workers continuously scan the queues for work. If a queue is temporarily empty, the worker takes a micro-pause (`std::this_thread::yield()`) to save power while staying ready to process incoming orders within nanoseconds.
Now that your ARCHITECTURE.md is simplified and ready to save, would you like me to walk you through updating your v1.0.0 release version tag or drafting your CV bullet points for this project next?
AI responses may include mistakes. Learn more
