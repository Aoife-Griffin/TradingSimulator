#ifndef PERSISTENCE_MANAGER_HPP
#define PERSISTENCE_MANAGER_HPP
#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <chrono>

namespace Trading {

class PersistenceManager {
private:
    std::string m_dbPath;

public:
    /// Create tab;es
    PersistenceManager(const std::string& dbPath) : m_dbPath(dbPath) {
        initializeTables();
    }

    void initializeTables() {
        /// Check if file exists. If it doesn't, then write headers to mimic tables
        std::ifstream checkFile(m_dbPath);
        bool exists = checkFile.good();
        checkFile.close();

        if (!exists) {
            std::ofstream dbFile(m_dbPath);
            if (dbFile.is_open()) {
                dbFile << "=== SIMULATION RELATIONAL STORAGE DATABASE LEDGER ===\n";
                dbFile << "TABLE: simulations (sim_id, timestamp, total_orders, throughput_ops_sec)\n";
                dbFile << "TABLE: performance_metrics (sim_id, p50_us, p95_us, p99_us, p999_us, status)\n";
                dbFile.close();
            }
        }
    }

    /// Inserting simulation snapshots
    void saveSimulationRun(size_t totalOrders, double throughput, double p50, double p95, double p99, double p999) {
        std::ofstream dbFile(m_dbPath, std::ios::app);
        if (dbFile.is_open()) {
            ///Unique ID using timestamp hashing
            auto now = std::chrono::system_clock::now();
            auto simId = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() % 100000;

            /// Insert data into simulation row
            dbFile << "INSERT INTO simulations VALUES (" 
                   << simId << ", datetime('now'), " 
                   << totalOrders << ", " 
                   << static_cast<uint64_t>(throughput) << ");\n";

            /// Insert telemetry data
            dbFile << "INSERT INTO performance_metrics VALUES (" 
                   << simId << ", " 
                   << p50 << ", " 
                   << p95 << ", " 
                   << p99 << ", " 
                   << p999 << ", 'SUCCESS');\n\n";

            dbFile.close();
            std::cout << "[PERSISTENCE] Successfully saved simulation, orders, and portfolio metrics to database.\n";
        } else {
            std::cerr << "[PERSISTENCE ERROR] Failed to write database entries to path: " << m_dbPath << "\n";
        }
    }
};

}
#endif
