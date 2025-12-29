# BlazeCSV

**Blazing fast, header-only CSV parser for C++23**

[![CI](https://github.com/juanchristensen/blazecsv/actions/workflows/ci.yml/badge.svg)](https://github.com/juanchristensen/blazecsv/actions/workflows/ci.yml)
[![Sanitizers](https://github.com/juanchristensen/blazecsv/actions/workflows/sanitizers.yml/badge.svg)](https://github.com/juanchristensen/blazecsv/actions/workflows/sanitizers.yml)
[![Lint](https://github.com/juanchristensen/blazecsv/actions/workflows/lint.yml/badge.svg)](https://github.com/juanchristensen/blazecsv/actions/workflows/lint.yml)
[![CodeQL](https://github.com/juanchristensen/blazecsv/actions/workflows/codeql.yml/badge.svg)](https://github.com/juanchristensen/blazecsv/actions/workflows/codeql.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![Header-only](https://img.shields.io/badge/header--only-yes-green.svg)](include/blazecsv/blazecsv.hpp)

## Features

- **13M+ rows/second** single-threaded parsing
- **SIMD-accelerated** delimiter detection (ARM NEON / x86 SSE2)
- **Zero-copy** field access with `std::string_view`
- **Compile-time error policies** - pay only for what you use
- **Flexible null handling** - empty, NA, N/A, null, etc.
- **`std::expected`-based** error handling (C++23)
- **Memory-mapped I/O** for optimal performance
- **Parallel parsing** for multi-core systems
- **Header-only** - just include and go

## Quick Start

```cpp
#include <blazecsv/blazecsv.hpp>
#include <iostream>

int main() {
    // Fast parsing (no error checking)
    blazecsv::TurboReader<7> reader("data.csv");

    reader.for_each([](const auto& fields) {
        std::cout << fields[0].view() << ": "
                  << fields[4].value_or(0.0) << "\n";
    });
}
```

## Installation

### Header-Only (Recommended)

Copy `include/blazecsv/blazecsv.hpp` to your project:

```cpp
#include "blazecsv.hpp"
```

### CMake FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(
    blazecsv
    GIT_REPOSITORY https://github.com/juanchristensen/blazecsv.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(blazecsv)

target_link_libraries(your_target PRIVATE blazecsv::blazecsv)
```

### CMake Subdirectory

```cmake
add_subdirectory(blazecsv)
target_link_libraries(your_target PRIVATE blazecsv::blazecsv)
```

## Benchmark Results

Parsing 1M rows of OHLCV stock data (7 columns, ~80 bytes/row):

| Library | Rows/sec | Relative |
|---------|----------|----------|
| **BlazeCSV TurboReader** | 13,200,000 | 1.00x |
| **BlazeCSV Parallel (4T)** | 14,500,000 | 1.10x |
| fast-cpp-csv-parser | 8,500,000 | 0.64x |
| csv-parser | 3,200,000 | 0.24x |
| csv2 | 2,800,000 | 0.21x |
| rapidcsv | 1,500,000 | 0.11x |

*Tested on Apple M2 Pro, Clang 19, -O3*

## API Reference

### Reader Types

| Type | Error Tracking | Null Detection | Use Case |
|------|----------------|----------------|----------|
| `TurboReader<N>` | None | None | Maximum performance |
| `CheckedReader<N>` | Basic | Standard | Production with basic validation |
| `SafeReader<N>` | Full | Lenient | Development, data exploration |
| `ParallelReader<N>` | None | None | Large files, multi-core |

### FieldRef Methods

```cpp
// String access
std::string_view view() const;          // Zero-copy view

// Type-safe parsing with std::expected
std::expected<T, ErrorCode> parse<T>() const;

// Parsing with default value
T value_or(T default_val) const;

// Optional for null-aware parsing
std::optional<T> as_optional<T, NullPolicy>() const;

// Null checking
bool is_null<NullPolicy>() const;
```

### Error Policies

```cpp
// No error checking (fastest)
using TurboReader = Reader<N, ',', NoErrorCheck, NoNullCheck>;

// Basic error tracking (line numbers)
using CheckedReader = Reader<N, ',', ErrorCheckBasic, NullStandard>;

// Full error tracking (line + column + type)
using SafeReader = Reader<N, ',', ErrorCheckFull, NullLenient>;
```

### Null Policies

| Policy | Empty | NA | N/A | null/NULL | none/- |
|--------|-------|-----|-----|-----------|--------|
| `NullStrict` | Yes | No | No | No | No |
| `NullStandard` | Yes | Yes | Yes | No | No |
| `NullLenient` | Yes | Yes | Yes | Yes | Yes |
| `NoNullCheck` | No | No | No | No | No |

## Examples

### Basic Parsing

```cpp
blazecsv::TurboReader<3> reader("data.csv");

// Access headers
for (const auto& h : reader.headers()) {
    std::cout << h << " ";
}

// Iterate rows
reader.for_each([](const auto& fields) {
    std::string_view name = fields[0].view();
    int age = fields[1].value_or(0);
    double score = fields[2].value_or(0.0);
});
```

### Error Handling with std::expected

```cpp
blazecsv::SafeReader<3> reader("data.csv");

reader.for_each([](const auto& fields) {
    auto result = fields[0].template parse<int>();

    if (result) {
        std::cout << "Value: " << *result << "\n";
    } else {
        std::cout << "Error: " << static_cast<int>(result.error()) << "\n";
    }
});
```

### Null-Aware Parsing

```cpp
blazecsv::SafeReader<3> reader("data.csv");

reader.for_each([](const auto& fields) {
    // Check for null before parsing
    if (fields[1].template is_null<blazecsv::NullLenient>()) {
        std::cout << "Value is null\n";
        return;
    }

    // Or use as_optional
    auto value = fields[1].template as_optional<double, blazecsv::NullLenient>();
    if (value) {
        std::cout << "Value: " << *value << "\n";
    }
});
```

### Parallel Parsing

```cpp
blazecsv::ParallelReader<7> reader("large_file.csv");

std::atomic<size_t> count{0};
std::atomic<double> sum{0};

reader.for_each_parallel([&](const auto& fields) {
    double value = fields[4].value_or(0.0);

    // Atomic update for aggregation
    double current = sum.load(std::memory_order_relaxed);
    while (!sum.compare_exchange_weak(current, current + value,
           std::memory_order_relaxed)) {}

    count.fetch_add(1, std::memory_order_relaxed);
});
```

### TSV Support

```cpp
// Tab-separated values
blazecsv::TsvTurboReader<5> reader("data.tsv");

// Or custom delimiter
blazecsv::Reader<5, '|', blazecsv::NoErrorCheck, blazecsv::NoNullCheck> reader("data.psv");
```

### Raw Pointer Access (Maximum Performance)

```cpp
blazecsv::TurboReader<3> reader("data.csv");

reader.for_each_raw([](const char** starts, const char** ends) {
    // Direct pointer access to field data
    std::string_view name(starts[0], ends[0] - starts[0]);

    int age;
    std::from_chars(starts[1], ends[1], age);
});
```

### Early Termination

```cpp
blazecsv::TurboReader<3> reader("data.csv");
size_t count = 0;

reader.for_each_until([&count](const auto& fields) {
    std::cout << fields[0].view() << "\n";
    return ++count < 10;  // Stop after 10 rows
});
```

## Platform Support

| Platform | SIMD | Status |
|----------|------|--------|
| macOS (Apple Silicon) | ARM NEON | Fully supported |
| macOS (Intel) | SSE2 | Fully supported |
| Linux (x64) | SSE2 | Fully supported |
| Linux (ARM64) | NEON | Fully supported |
| Windows (x64) | SSE2 | Should work (untested) |

## Requirements

- C++23 compiler (Clang 17+, GCC 13+, MSVC 2022+)
- For parallel parsing: `<thread>` support

## Building from Source

```bash
# Configure with examples and tests
cmake -B build -DBLAZECSV_BUILD_EXAMPLES=ON -DBLAZECSV_BUILD_TESTS=ON

# Build
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure

# Run examples
./build/examples/basic
./build/examples/trading_data
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

MIT License - see [LICENSE](LICENSE) for details.

## Acknowledgments

- SIMD techniques inspired by various high-performance parsing libraries
- Memory-mapped I/O patterns from modern systems programming
