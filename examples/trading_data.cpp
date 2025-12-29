// BlazeCSV - Trading Data Example
//
// Demonstrates parsing OHLCV (Open, High, Low, Close, Volume) stock data

#include <blazecsv/blazecsv.hpp>

#include <chrono>
#include <fstream>
#include <iostream>

// Trade record structure
struct Trade {
    std::chrono::year_month_day date;
    double open;
    double high;
    double low;
    double close;
    int64_t volume;
};

int main() {
    // Create sample OHLCV data
    const std::string filename = "/tmp/ohlcv.csv";
    {
        std::ofstream f(filename);
        f << "Date,Open,High,Low,Close,Volume\n";
        f << "2024-01-02,185.50,186.75,184.25,186.00,50000000\n";
        f << "2024-01-03,186.00,188.50,185.50,187.75,48000000\n";
        f << "2024-01-04,187.75,189.00,186.00,188.50,52000000\n";
        f << "2024-01-05,188.50,190.25,188.00,189.75,55000000\n";
        f << "2024-01-08,189.75,191.00,189.00,190.50,47000000\n";
    }

    std::cout << "=== BlazeCSV Trading Data Example ===\n\n";

    // Parse OHLCV data with date parsing
    std::cout << "1. Parsing OHLCV data with date parsing:\n";
    {
        blazecsv::TurboReader<6> reader(filename);

        // Show column names
        std::cout << "   Columns: ";
        for (const auto& h : reader.headers()) {
            std::cout << h << " ";
        }
        std::cout << "\n\n";

        // Parse and display
        reader.for_each([](const auto& fields) {
            // Parse date
            auto date_result = fields[0].parse_date();
            if (!date_result) {
                std::cerr << "   Invalid date!\n";
                return;
            }
            auto ymd = *date_result;

            // Parse OHLCV values
            double open = fields[1].value_or(0.0);
            double high = fields[2].value_or(0.0);
            double low = fields[3].value_or(0.0);
            double close = fields[4].value_or(0.0);
            int64_t volume = fields[5].template value_or<int64_t>(0);

            // Calculate daily range
            double range = high - low;
            double change_pct = ((close - open) / open) * 100.0;

            std::cout << "   " << static_cast<int>(ymd.year()) << "-"
                      << static_cast<unsigned>(ymd.month()) << "-"
                      << static_cast<unsigned>(ymd.day()) << ": O=" << open << " H=" << high
                      << " L=" << low << " C=" << close << " V=" << volume << " | Range: $" << range
                      << " | Change: " << (change_pct >= 0 ? "+" : "") << change_pct << "%\n";
        });
    }

    // Calculate statistics
    std::cout << "\n2. Calculating statistics:\n";
    {
        blazecsv::TurboReader<6> reader(filename);

        double total_volume = 0;
        double max_high = 0;
        double min_low = 1e10;
        size_t count = 0;

        reader.for_each([&](const auto& fields) {
            double high = fields[2].value_or(0.0);
            double low = fields[3].value_or(0.0);
            int64_t volume = fields[5].template value_or<int64_t>(0);

            if (high > max_high)
                max_high = high;
            if (low < min_low)
                min_low = low;
            total_volume += volume;
            ++count;
        });

        std::cout << "   Total records: " << count << "\n";
        std::cout << "   Period high: $" << max_high << "\n";
        std::cout << "   Period low: $" << min_low << "\n";
        std::cout << "   Total volume: " << static_cast<int64_t>(total_volume) << "\n";
        std::cout << "   Avg daily volume: " << static_cast<int64_t>(total_volume / count) << "\n";
    }

    // Performance test
    std::cout << "\n3. Performance test (parsing 1000x):\n";
    {
        auto start = std::chrono::high_resolution_clock::now();

        size_t total_records = 0;
        for (int i = 0; i < 1000; ++i) {
            blazecsv::TurboReader<6> reader(filename);
            total_records += reader.for_each([](const auto&) {});
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "   Parsed " << total_records << " records in " << duration_ms << " ms\n";
        std::cout << "   Rate: " << (total_records / duration_ms * 1000) << " records/sec\n";
    }

    // Clean up
    std::remove(filename.c_str());

    std::cout << "\n=== Done! ===\n";
    return 0;
}
