// BlazeCSV - Parallel Parsing Example
//
// Demonstrates multi-threaded CSV parsing for large files

#include <blazecsv/blazecsv.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <atomic>
#include <thread>

int main() {
    // Create a larger CSV file for parallel processing demo
    const std::string filename = "/tmp/large_data.csv";
    const size_t num_rows = 100000;

    std::cout << "=== BlazeCSV Parallel Parsing Example ===\n\n";

    // Generate test data
    std::cout << "1. Generating " << num_rows << " rows of test data...\n";
    {
        std::ofstream f(filename);
        f << "id,symbol,price,quantity,side\n";
        for (size_t i = 0; i < num_rows; ++i) {
            f << i << ",AAPL," << (150.0 + (i % 100) * 0.01) << ","
              << (100 + i % 1000) << "," << (i % 2 ? "BUY" : "SELL") << "\n";
        }
    }
    std::cout << "   Done!\n\n";

    // Single-threaded baseline
    std::cout << "2. Single-threaded parsing (TurboReader):\n";
    {
        auto start = std::chrono::high_resolution_clock::now();

        blazecsv::TurboReader<5> reader(filename);

        double total_value = 0;
        size_t count = reader.for_each([&total_value](const auto& fields) {
            double price = fields[2].value_or(0.0);
            int64_t qty = fields[3].template value_or<int64_t>(0);
            total_value += price * qty;
        });

        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "   Processed: " << count << " rows\n";
        std::cout << "   Total value: $" << static_cast<int64_t>(total_value) << "\n";
        std::cout << "   Time: " << duration_ms << " ms\n";
        std::cout << "   Rate: " << (count / duration_ms * 1000) << " rows/sec\n";
    }

    // Parallel parsing
    std::cout << "\n3. Parallel parsing (ParallelReader with "
              << std::thread::hardware_concurrency() << " threads):\n";
    {
        auto start = std::chrono::high_resolution_clock::now();

        blazecsv::ParallelReader<5> reader(filename);

        std::atomic<double> total_value{0};
        std::atomic<size_t> total_count{0};

        reader.for_each_parallel([&](const auto& fields) {
            double price = fields[2].value_or(0.0);
            int64_t qty = fields[3].template value_or<int64_t>(0);

            // Use relaxed atomic add for aggregation
            double current = total_value.load(std::memory_order_relaxed);
            while (!total_value.compare_exchange_weak(
                current, current + price * qty,
                std::memory_order_relaxed, std::memory_order_relaxed)) {}

            total_count.fetch_add(1, std::memory_order_relaxed);
        });

        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "   Processed: " << total_count.load() << " rows\n";
        std::cout << "   Total value: $" << static_cast<int64_t>(total_value.load()) << "\n";
        std::cout << "   Time: " << duration_ms << " ms\n";
        std::cout << "   Rate: " << (total_count.load() / duration_ms * 1000) << " rows/sec\n";
    }

    // Parallel with thread-local accumulation (better performance)
    std::cout << "\n4. Parallel with thread-local accumulation:\n";
    {
        auto start = std::chrono::high_resolution_clock::now();

        blazecsv::ParallelReader<5> reader(filename);

        // Thread-local storage for better performance
        struct ThreadLocal {
            double value{0};
            size_t count{0};
        };
        std::vector<ThreadLocal> thread_data(std::thread::hardware_concurrency());

        // Get thread index from thread ID
        std::atomic<size_t> thread_counter{0};
        thread_local size_t my_thread_id = thread_counter.fetch_add(1);

        reader.for_each_parallel([&](const auto& fields) {
            size_t tid = my_thread_id % thread_data.size();
            double price = fields[2].value_or(0.0);
            int64_t qty = fields[3].template value_or<int64_t>(0);
            thread_data[tid].value += price * qty;
            thread_data[tid].count++;
        });

        // Aggregate results
        double total_value = 0;
        size_t total_count = 0;
        for (const auto& td : thread_data) {
            total_value += td.value;
            total_count += td.count;
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "   Processed: " << total_count << " rows\n";
        std::cout << "   Total value: $" << static_cast<int64_t>(total_value) << "\n";
        std::cout << "   Time: " << duration_ms << " ms\n";
        std::cout << "   Rate: " << (total_count / duration_ms * 1000) << " rows/sec\n";
    }

    // Parallel with custom thread count
    std::cout << "\n5. Parallel with custom thread count (2 threads):\n";
    {
        auto start = std::chrono::high_resolution_clock::now();

        blazecsv::ParallelReader<5> reader(filename, 2);  // 2 threads

        std::atomic<size_t> count{0};

        reader.for_each_parallel([&](const auto&) {
            count.fetch_add(1, std::memory_order_relaxed);
        });

        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "   Processed: " << count.load() << " rows\n";
        std::cout << "   Time: " << duration_ms << " ms\n";
        std::cout << "   Rate: " << (count.load() / duration_ms * 1000) << " rows/sec\n";
    }

    // Clean up
    std::remove(filename.c_str());

    std::cout << "\n=== Done! ===\n";
    return 0;
}
