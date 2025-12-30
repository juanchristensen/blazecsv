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
- **Compile-time error policies** — pay only for what you use
- **Flexible null handling** — empty, NA, N/A, null, etc.
- **`std::expected`-based** error handling (C++23)
- **Memory-mapped I/O** for optimal performance
- **Parallel parsing** for multi-core systems
- **Cross-platform** — Linux, macOS, Windows
- **Header-only** — just include and go

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

## Performance

Run the included benchmark to measure performance on your system:

```bash
cmake -B build -DBLAZECSV_BUILD_BENCHMARKS=ON
cmake --build build
./build/benchmark/blazecsv_bench
```

Example output (Apple M2 Pro):

```
=== BlazeCSV Performance Benchmark ===

System: 12 threads available

--- Large File (1,000,000 rows) ---
  TurboReader:      76.2 ms  |     13,119,533 rows/sec
  CheckedReader:    76.8 ms  |     13,020,833 rows/sec
  SafeReader:       82.1 ms  |     12,180,267 rows/sec
  for_each_raw:     71.4 ms  |     14,005,602 rows/sec
  ParallelReader:   68.9 ms  |     14,513,788 rows/sec
```

## API Reference

### Reader Types

| Type | Error Tracking | Null Detection | Use Case |
|------|----------------|----------------|----------|
| `TurboReader<N>` | None | None | Maximum performance |
| `CheckedReader<N>` | Basic | Standard | Production with validation |
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

See the `examples/` directory for complete working examples:

- **basic.cpp** — Simple parsing and field access
- **error_handling.cpp** — Using `std::expected` and error policies
- **parallel.cpp** — Multi-threaded parsing for large files
- **trading_data.cpp** — OHLCV stock data with date parsing

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

### TSV and Custom Delimiters

```cpp
// Tab-separated values
blazecsv::TsvTurboReader<5> reader("data.tsv");

// Custom delimiter (pipe)
blazecsv::Reader<5, '|', blazecsv::NoErrorCheck, blazecsv::NoNullCheck> reader("data.psv");
```

### Raw Pointer Access

For maximum performance when you need direct memory access:

```cpp
blazecsv::TurboReader<3> reader("data.csv");

reader.for_each_raw([](const char** starts, const char** ends) {
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

| Platform | Architecture | SIMD | Status |
|----------|--------------|------|--------|
| Linux | x86_64 | SSE2 | Fully supported |
| Linux | ARM64 | NEON | Fully supported |
| macOS | Apple Silicon | NEON | Fully supported |
| macOS | Intel | SSE2 | Fully supported |
| Windows | x86_64 | SSE2 | Fully supported |

## Requirements

- C++23 compiler:
  - GCC 13+
  - Clang 17+
  - MSVC 2022 (19.36+)
- For parallel parsing: `<thread>` support

## Building from Source

```bash
# Configure with all options
cmake -B build \
    -DBLAZECSV_BUILD_EXAMPLES=ON \
    -DBLAZECSV_BUILD_TESTS=ON \
    -DBLAZECSV_BUILD_BENCHMARKS=ON

# Build
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure

# Run examples
./build/examples/basic
./build/examples/trading_data
./build/examples/error_handling
./build/examples/parallel

# Run benchmark
./build/benchmark/blazecsv_bench
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

MIT License — see [LICENSE](LICENSE) for details.
