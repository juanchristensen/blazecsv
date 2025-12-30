// BlazeCSV Self-Benchmark
//
// Performance test showcasing different reader types and access patterns

#include <blazecsv/blazecsv.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>

// Cross-platform temp file path
inline std::string temp_path(const std::string& name) {
    return (std::filesystem::temp_directory_path() / name).string();
}

constexpr size_t SMALL_ROWS = 100'000;
constexpr size_t LARGE_ROWS = 1'000'000;

void generate_csv(const std::string& filename, size_t rows) {
    std::ofstream f(filename);
    f << "Date,Open,High,Low,Close,Volume,Symbol\n";
    for (size_t i = 0; i < rows; ++i) {
        double base = 150.0 + (i % 100);
        f << "2024-01-" << std::setw(2) << std::setfill('0') << ((i % 28) + 1) << "," << std::fixed
          << std::setprecision(2) << base << "," << (base + 2.5) << "," << (base - 1.5) << ","
          << (base + 0.75) << "," << (1000000 + i * 100) << ",AAPL\n";
    }
}

template <typename Func>
double time_ms(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

void bench_turbo_reader(const std::string& file, size_t expected_rows) {
    size_t rows = 0;
    double sum = 0;

    double t = time_ms([&]() {
        blazecsv::TurboReader<7> reader(file);
        reader.for_each([&](const auto& fields) {
            sum += fields[4].value_or(0.0);
            ++rows;
        });
    });

    std::cout << "  TurboReader:   " << std::setw(8) << std::fixed << std::setprecision(1) << t
              << " ms  |  " << std::setprecision(0) << std::setw(12) << (rows / t * 1000)
              << " rows/sec\n";
}

void bench_checked_reader(const std::string& file, size_t expected_rows) {
    size_t rows = 0;
    double sum = 0;

    double t = time_ms([&]() {
        blazecsv::CheckedReader<7> reader(file);
        rows = reader.for_each([&](const auto& fields) { sum += fields[4].value_or(0.0); });
    });

    std::cout << "  CheckedReader: " << std::setw(8) << std::fixed << std::setprecision(1) << t
              << " ms  |  " << std::setprecision(0) << std::setw(12) << (rows / t * 1000)
              << " rows/sec\n";
}

void bench_safe_reader(const std::string& file, size_t expected_rows) {
    size_t rows = 0;
    double sum = 0;

    double t = time_ms([&]() {
        blazecsv::SafeReader<7> reader(file);
        reader.for_each([&](const auto& fields) {
            auto result = fields[4].template parse<double>();
            if (result)
                sum += *result;
            ++rows;
        });
    });

    std::cout << "  SafeReader:    " << std::setw(8) << std::fixed << std::setprecision(1) << t
              << " ms  |  " << std::setprecision(0) << std::setw(12) << (rows / t * 1000)
              << " rows/sec\n";
}

void bench_parallel_reader(const std::string& file, size_t expected_rows) {
    std::atomic<size_t> rows{0};

    double t = time_ms([&]() {
        blazecsv::ParallelReader<7> reader(file);
        reader.for_each_parallel(
            [&](const auto&) { rows.fetch_add(1, std::memory_order_relaxed); });
    });

    std::cout << "  ParallelReader (" << std::thread::hardware_concurrency()
              << "T): " << std::setw(5) << std::fixed << std::setprecision(1) << t << " ms  |  "
              << std::setprecision(0) << std::setw(12) << (rows.load() / t * 1000) << " rows/sec\n";
}

void bench_raw_access(const std::string& file, size_t expected_rows) {
    size_t rows = 0;
    double sum = 0;

    double t = time_ms([&]() {
        blazecsv::TurboReader<7> reader(file);
        reader.for_each_raw([&](const char** starts, const char** ends) {
            // Direct parsing on close price
            sum += std::strtod(starts[4], nullptr);
            ++rows;
        });
    });

    std::cout << "  for_each_raw:  " << std::setw(8) << std::fixed << std::setprecision(1) << t
              << " ms  |  " << std::setprecision(0) << std::setw(12) << (rows / t * 1000)
              << " rows/sec\n";
}

int main() {
    std::cout << "=== BlazeCSV Performance Benchmark ===\n\n";
    std::cout << "System: " << std::thread::hardware_concurrency() << " threads available\n\n";

    // Small file test
    const std::string small_file = temp_path("bench_small.csv");
    std::cout << "Generating " << SMALL_ROWS << " rows...\n";
    generate_csv(small_file, SMALL_ROWS);

    std::cout << "\n--- Small File (" << SMALL_ROWS << " rows) ---\n";
    bench_turbo_reader(small_file, SMALL_ROWS);
    bench_checked_reader(small_file, SMALL_ROWS);
    bench_safe_reader(small_file, SMALL_ROWS);
    bench_raw_access(small_file, SMALL_ROWS);
    bench_parallel_reader(small_file, SMALL_ROWS);

    // Large file test
    const std::string large_file = temp_path("bench_large.csv");
    std::cout << "\nGenerating " << LARGE_ROWS << " rows...\n";
    generate_csv(large_file, LARGE_ROWS);

    std::cout << "\n--- Large File (" << LARGE_ROWS << " rows) ---\n";
    bench_turbo_reader(large_file, LARGE_ROWS);
    bench_checked_reader(large_file, LARGE_ROWS);
    bench_safe_reader(large_file, LARGE_ROWS);
    bench_raw_access(large_file, LARGE_ROWS);
    bench_parallel_reader(large_file, LARGE_ROWS);

    // Cleanup
    std::remove(small_file.c_str());
    std::remove(large_file.c_str());

    std::cout << "\n=== Done! ===\n";
    return 0;
}
