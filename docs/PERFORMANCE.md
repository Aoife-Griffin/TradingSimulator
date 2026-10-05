# Performance Engineering & Benchmarking Metrics

## 1. Live Platform Production Telemetry (20,000,000 Orders)
The following speed metrics were recorded running an optimized Release setup on a multi-core Windows machine under a heavy load of 1,000 automated trading bots and active chaos engineering fault injections:

* **Peak Platform Throughput:** 1,072,830 orders per second
* **Average Latency:** 0.49 microseconds
* **p50 (Median) Latency:** 0.40 microseconds
* **p95 Latency:** 0.80 microseconds
* **p99 Latency:** 1.10 microseconds
* **p99.9 (Tail) Latency:** 4.20 microseconds

## 2. Memory Layout Experiment: AOS vs. SOA
To measure how memory layouts affect data speeds during massive analytical sweeps, a standalone test evaluated an Array of Structures (AOS) against a Structure of Arrays (SOA) across 5,000,000 historical rows:
* **AOS Sequential Processing Time:** 21,538 us
* **SOA Contiguous Vectorization Time:** 9,630 us
* **Hardware Performance Difference:** **The SOA layout ran 2.24x faster than AoS.**
* **Architectural Takeaway:** Grouping matching number categories together into separate lists prevents cache line clutter. The CPU reads pure columns of values instantly, allowing the compiler to use vectorized hardware loops to process multiple items at the exact same time.
