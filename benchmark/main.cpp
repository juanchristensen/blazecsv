// BlazeCSV Benchmark - Comparison with Other Libraries
//
// Compares BlazeCSV against popular CSV parsing libraries

#include <blazecsv/blazecsv.hpp>

// External libraries
#include <csv.h>            // ben-strasser/fast-cpp-csv-parser
#include <csv.hpp>          // vincentlaucsb/csv-parser
#include <csv2/reader.hpp>  // p-ranav/csv2
#include <rapidcsv.h>       // d99kris/rapidcsv

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

// Benchmark configuration
constexpr size_t NUM_ROWS = 1'000'000;
constexpr size_t NUM_COLS = 7;
constexpr int WARMUP_RUNS = 2;
constexpr int BENCH_RUNS = 5;

const std::string BENCHMARK_FILE = "/tmp/benchmark_data.csv";

struct BenchResult {
    std::string name;
    double time_ms;
    size_t rows;
    double rate;  // rows/sec
};

void generate_benchmark_data() {
    std::cout << "Generating " << NUM_ROWS << " rows of benchmark data...\n";
    std::ofstream f(BENCHMARK_FILE);

    // Header
    f << "Date,Open,High,Low,Close,Volume,Symbol\n";

    // Data rows
    for (size_t i = 0; i < NUM_ROWS; ++i) {
        double base = 150.0 + (i % 100);
        f << "2024-01-" << std::setw(2) << std::setfill('0') << ((i % 28) + 1) << "," << std::fixed
          << std::setprecision(2) << base << "," << (base + 2.5) << "," << (base - 1.5) << ","
          << (base + 0.75) << "," << (1000000 + i * 100) << ","
          << "AAPL\n";
    }

    std::cout << "Generated " << (std::filesystem::file_size(BENCHMARK_FILE) / 1024 / 1024)
              << " MB of data\n\n";
}

template <typename Func>
double benchmark(Func&& func, int runs = BENCH_RUNS) {
    // Warmup
    for (int i = 0; i < WARMUP_RUNS; ++i) {
        func();
    }

    // Timed runs
    double total_ms = 0;
    for (int i = 0; i < runs; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        total_ms += std::chrono::duration<double, std::milli>(end - start).count();
    }

    return total_ms / runs;
}

BenchResult bench_blazecsv_turbo() {
    size_t rows = 0;
    double sum = 0;

    double time_ms = benchmark([&]() {
        blazecsv::TurboReader<NUM_COLS> reader(BENCHMARK_FILE);
        rows = 0;
        sum = 0;
        reader.for_each([&](const auto& fields) {
            sum += fields[4].value_or(0.0);  // Close price
            ++rows;
        });
    });

    return {"BlazeCSV TurboReader", time_ms, rows, rows / time_ms * 1000};
}

BenchResult bench_blazecsv_safe() {
    size_t rows = 0;
    double sum = 0;

    double time_ms = benchmark([&]() {
        blazecsv::SafeReader<NUM_COLS> reader(BENCHMARK_FILE);
        rows = 0;
        sum = 0;
        reader.for_each([&](const auto& fields) {
            sum += fields[4].value_or(0.0);
            ++rows;
        });
    });

    return {"BlazeCSV SafeReader", time_ms, rows, rows / time_ms * 1000};
}

BenchResult bench_blazecsv_parallel() {
    std::atomic<size_t> rows{0};
    std::atomic<double> sum{0};

    double time_ms = benchmark([&]() {
        blazecsv::ParallelReader<NUM_COLS> reader(BENCHMARK_FILE);
        rows = 0;
        sum = 0;
        reader.for_each_parallel([&](const auto& fields) {
            double val = fields[4].value_or(0.0);
            // Relaxed atomic for aggregation
            double current = sum.load(std::memory_order_relaxed);
            while (!sum.compare_exchange_weak(current, current + val, std::memory_order_relaxed)) {}
            rows.fetch_add(1, std::memory_order_relaxed);
        });
    });

    return {"BlazeCSV Parallel (" + std::to_string(std::thread::hardware_concurrency()) + "T)",
            time_ms, rows.load(), rows.load() / time_ms * 1000};
}

BenchResult bench_csv_parser() {
    size_t rows = 0;
    double sum = 0;

    double time_ms = benchmark([&]() {
        csv::CSVReader reader(BENCHMARK_FILE);
        rows = 0;
        sum = 0;
        for (csv::CSVRow& row : reader) {
            sum += row["Close"].get<double>();
            ++rows;
        }
    });

    return {"csv-parser (vincentlaucsb)", time_ms, rows, rows / time_ms * 1000};
}

BenchResult bench_csv2() {
    size_t rows = 0;
    double sum = 0;

    double time_ms = benchmark([&]() {
        csv2::Reader<csv2::delimiter<','>, csv2::quote_character<'"'>,
                     csv2::first_row_is_header<true>>
            reader;
        if (reader.mmap(BENCHMARK_FILE)) {
            rows = 0;
            sum = 0;
            for (const auto& row : reader) {
                int col = 0;
                for (const auto& cell : row) {
                    if (col == 4) {  // Close column
                        std::string val;
                        cell.read_value(val);
                        sum += std::stod(val);
                    }
                    ++col;
                }
                ++rows;
            }
        }
    });

    return {"csv2 (p-ranav)", time_ms, rows, rows / time_ms * 1000};
}

BenchResult bench_rapidcsv() {
    size_t rows = 0;
    double sum = 0;

    double time_ms = benchmark([&]() {
        rapidcsv::Document doc(BENCHMARK_FILE);
        rows = doc.GetRowCount();
        sum = 0;
        for (size_t i = 0; i < rows; ++i) {
            sum += doc.GetCell<double>("Close", i);
        }
    });

    return {"rapidcsv (d99kris)", time_ms, rows, rows / time_ms * 1000};
}

BenchResult bench_fast_csv() {
    size_t rows = 0;
    double sum = 0;

    double time_ms = benchmark([&]() {
        io::CSVReader<7> reader(BENCHMARK_FILE);
        reader.read_header(io::ignore_extra_column, "Date", "Open", "High", "Low", "Close",
                           "Volume", "Symbol");

        std::string date, symbol;
        double open, high, low, close;
        int64_t volume;

        rows = 0;
        sum = 0;
        while (reader.read_row(date, open, high, low, close, volume, symbol)) {
            sum += close;
            ++rows;
        }
    });

    return {"fast-cpp-csv-parser (ben-strasser)", time_ms, rows, rows / time_ms * 1000};
}

void print_results(const std::vector<BenchResult>& results) {
    // Find fastest for relative comparison
    double fastest = results[0].rate;
    for (const auto& r : results) {
        if (r.rate > fastest)
            fastest = r.rate;
    }

    std::cout << "\n";
    std::cout << std::left << std::setw(40) << "Library" << std::right << std::setw(12)
              << "Time (ms)" << std::setw(15) << "Rows/sec" << std::setw(12) << "Relative"
              << "\n";
    std::cout << std::string(79, '-') << "\n";

    for (const auto& r : results) {
        std::cout << std::left << std::setw(40) << r.name << std::right << std::setw(12)
                  << std::fixed << std::setprecision(1) << r.time_ms << std::setw(15)
                  << std::setprecision(0) << r.rate << std::setw(11) << std::setprecision(2)
                  << (r.rate / fastest) << "x\n";
    }
}

int main() {
    std::cout << "=== BlazeCSV Benchmark Suite ===\n\n";
    std::cout << "Benchmark config:\n";
    std::cout << "  Rows: " << NUM_ROWS << "\n";
    std::cout << "  Columns: " << NUM_COLS << "\n";
    std::cout << "  Warmup runs: " << WARMUP_RUNS << "\n";
    std::cout << "  Benchmark runs: " << BENCH_RUNS << "\n";
    std::cout << "  Threads available: " << std::thread::hardware_concurrency() << "\n\n";

    generate_benchmark_data();

    std::cout << "Running benchmarks...\n";

    std::vector<BenchResult> results;

    std::cout << "  BlazeCSV TurboReader... ";
    results.push_back(bench_blazecsv_turbo());
    std::cout << "done\n";

    std::cout << "  BlazeCSV SafeReader... ";
    results.push_back(bench_blazecsv_safe());
    std::cout << "done\n";

    std::cout << "  BlazeCSV Parallel... ";
    results.push_back(bench_blazecsv_parallel());
    std::cout << "done\n";

    std::cout << "  csv-parser... ";
    results.push_back(bench_csv_parser());
    std::cout << "done\n";

    std::cout << "  csv2... ";
    results.push_back(bench_csv2());
    std::cout << "done\n";

    std::cout << "  rapidcsv... ";
    results.push_back(bench_rapidcsv());
    std::cout << "done\n";

    std::cout << "  fast-cpp-csv-parser... ";
    results.push_back(bench_fast_csv());
    std::cout << "done\n";

    print_results(results);

    // Clean up
    std::remove(BENCHMARK_FILE.c_str());

    std::cout << "\n=== Done! ===\n";
    return 0;
}
