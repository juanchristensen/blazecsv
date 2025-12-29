// BlazeCSV - Basic Usage Example
//
// Demonstrates the simplest way to use BlazeCSV

#include <blazecsv/blazecsv.hpp>
#include <iostream>
#include <fstream>

int main() {
    // Create a sample CSV file
    const std::string filename = "/tmp/sample.csv";
    {
        std::ofstream f(filename);
        f << "name,age,score\n";
        f << "Alice,30,95.5\n";
        f << "Bob,25,87.2\n";
        f << "Charlie,35,92.0\n";
    }

    std::cout << "=== BlazeCSV Basic Example ===\n\n";

    // Method 1: TurboReader (maximum performance)
    std::cout << "1. Using TurboReader (maximum performance):\n";
    {
        blazecsv::TurboReader<3> reader(filename);

        // Access header names
        std::cout << "   Headers: ";
        for (const auto& h : reader.headers()) {
            std::cout << h << " ";
        }
        std::cout << "\n";

        // Iterate over rows
        reader.for_each([](const auto& fields) {
            std::cout << "   " << fields[0].view() << ": "
                      << fields[1].value_or(0) << " years, "
                      << "score = " << fields[2].value_or(0.0) << "\n";
        });
    }

    // Method 2: Using for_each with raw pointers (even faster)
    std::cout << "\n2. Using for_each_raw (raw pointers):\n";
    {
        blazecsv::TurboReader<3> reader(filename);

        reader.for_each_raw([](const char** starts, const char** ends) {
            // Direct access to field data
            std::string_view name(starts[0], ends[0] - starts[0]);
            int age = 0;
            std::from_chars(starts[1], ends[1], age);
            double score = std::strtod(starts[2], nullptr);

            std::cout << "   " << name << ": " << age << " years, score = " << score << "\n";
        });
    }

    // Method 3: Early termination
    std::cout << "\n3. Early termination (first 2 rows only):\n";
    {
        blazecsv::TurboReader<3> reader(filename);
        size_t count = 0;

        reader.for_each_until([&count](const auto& fields) {
            std::cout << "   " << fields[0].view() << "\n";
            return ++count < 2;  // Stop after 2 rows
        });

        std::cout << "   (stopped after " << count << " rows)\n";
    }

    // Clean up
    std::remove(filename.c_str());

    std::cout << "\n=== Done! ===\n";
    return 0;
}
