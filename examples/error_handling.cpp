// BlazeCSV - Error Handling Example
//
// Demonstrates compile-time error policies and std::expected

#include <blazecsv/blazecsv.hpp>

#include <filesystem>

// Cross-platform temp file path
inline std::string temp_path(const std::string& name) {
    return (std::filesystem::temp_directory_path() / name).string();
}

#include <fstream>
#include <iostream>

int main() {
    // Create a CSV with some problematic data
    const std::string filename = temp_path("errors.csv");
    {
        std::ofstream f(filename);
        f << "id,name,value,active\n";
        f << "1,Alice,100.5,true\n";
        f << "2,Bob,invalid,false\n";  // Invalid number
        f << "3,Charlie,,yes\n";       // Empty value (null)
        f << "4,Diana,200.0,maybe\n";  // Invalid boolean
        f << "5,Eve,NA,1\n";           // NA value (null)
        f << "6,Frank,300.0\n";        // Missing column
    }

    std::cout << "=== BlazeCSV Error Handling Example ===\n\n";

    // Method 1: TurboReader (no error checking - fastest)
    std::cout << "1. TurboReader (no error checking):\n";
    {
        blazecsv::TurboReader<4> reader(filename);

        reader.for_each([](const auto& fields) {
            // Parsing errors return default values silently
            int id = fields[0].value_or(-1);
            auto name = fields[1].view();
            double value = fields[2].value_or(0.0);
            bool active = fields[3].value_or(false);

            std::cout << "   " << id << ": " << name << " = " << value
                      << " (active: " << std::boolalpha << active << ")\n";
        });
        std::cout << "   (Malformed rows may produce unexpected results)\n";
    }

    // Method 2: SafeReader (full error tracking)
    std::cout << "\n2. SafeReader (full error tracking):\n";
    {
        blazecsv::SafeReader<4> reader(filename);

        reader.for_each([](const auto& fields) {
            // Use std::expected for explicit error handling
            auto id_result = fields[0].template parse<int>();
            auto value_result = fields[2].template parse<double>();
            auto active_result = fields[3].template parse<bool>();

            std::cout << "   Row: ";

            if (id_result) {
                std::cout << "id=" << *id_result;
            } else {
                std::cout << "id=ERROR(" << static_cast<int>(id_result.error()) << ")";
            }

            std::cout << ", name=" << fields[1].view();

            // Check for null before parsing
            if (fields[2].template is_null<blazecsv::NullLenient>()) {
                std::cout << ", value=NULL";
            } else if (value_result) {
                std::cout << ", value=" << *value_result;
            } else {
                std::cout << ", value=PARSE_ERROR";
            }

            if (active_result) {
                std::cout << ", active=" << std::boolalpha << *active_result;
            } else {
                std::cout << ", active=INVALID";
            }

            std::cout << "\n";
        });

        if (reader.has_error()) {
            std::cout << "   Last error: code=" << static_cast<int>(reader.last_error().code)
                      << " line=" << reader.last_error().line << "\n";
        }
    }

    // Method 3: Using as_optional for null-aware parsing
    std::cout << "\n3. Null-aware parsing with as_optional:\n";
    {
        blazecsv::SafeReader<4> reader(filename);

        reader.for_each([](const auto& fields) {
            int id = fields[0].value_or(-1);
            auto name = fields[1].view();

            // as_optional returns nullopt for null values or parse errors
            auto value = fields[2].template as_optional<double, blazecsv::NullLenient>();
            auto active = fields[3].template as_optional<bool>();

            std::cout << "   " << id << ": " << name << " = ";
            if (value) {
                std::cout << *value;
            } else {
                std::cout << "N/A";
            }
            std::cout << " (active: ";
            if (active) {
                std::cout << std::boolalpha << *active;
            } else {
                std::cout << "unknown";
            }
            std::cout << ")\n";
        });
    }

    // Method 4: CheckedReader (error tracking without null handling)
    std::cout << "\n4. CheckedReader (line tracking only):\n";
    {
        blazecsv::CheckedReader<4> reader(filename);

        size_t processed = reader.for_each([](const auto&) {
            // Just count valid rows
        });

        std::cout << "   Processed " << processed << " valid rows\n";

        if (reader.has_error()) {
            std::cout << "   Skipped rows with column count mismatch\n";
            std::cout << "   Last error at line " << reader.last_error().line << "\n";
        }
    }

    // Clean up
    std::remove(filename.c_str());

    std::cout << "\n=== Done! ===\n";
    return 0;
}
