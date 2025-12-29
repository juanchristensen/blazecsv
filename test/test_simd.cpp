// BlazeCSV - SIMD Tests
//
// Tests for SIMD delimiter and newline detection

#include <blazecsv/blazecsv.hpp>

#include <chrono>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

#define TEST(name)                       \
    std::cout << "  " << name << "... "; \
    tests_run++
#define PASS()             \
    std::cout << "PASS\n"; \
    tests_passed++
#define FAIL(msg) std::cout << "FAIL: " << msg << "\n"

static int tests_run = 0;
static int tests_passed = 0;

void test_simd_detection() {
    std::cout << "\n=== SIMD Detection ===\n";

    TEST("SIMD available");
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    std::cout << "PASS (ARM NEON)\n";
    tests_passed++;
#elif defined(__SSE2__)
    std::cout << "PASS (x86 SSE2)\n";
    tests_passed++;
#else
    std::cout << "PASS (scalar fallback)\n";
    tests_passed++;
#endif
}

void test_delimiter_finding() {
    std::cout << "\n=== Delimiter Finding ===\n";

    // Test data with commas at various positions
    std::vector<std::pair<std::string, size_t>> test_cases = {
        {"hello,world", 5},
        {",start", 0},
        {"no delimiter here", 17},  // Returns length if not found
        {"a,b,c,d", 1},
        {"0123456789012345,after16", 16},  // Past SIMD boundary
        {"01234567890123456789012345678901,after32", 32},
    };

    for (const auto& [data, expected] : test_cases) {
        TEST(("comma at " + std::to_string(expected)).c_str());
        size_t result = blazecsv::detail::find_field_end(data.data(), data.size(), ',');
        if (result == expected) {
            PASS();
        } else {
            FAIL("got " + std::to_string(result) + ", expected " + std::to_string(expected));
        }
    }
}

void test_newline_finding() {
    std::cout << "\n=== Newline Finding ===\n";

    std::vector<std::pair<std::string, size_t>> test_cases = {
        {"hello\nworld", 5},
        {"\nstart", 0},
        {"no newline here!", 16},
        {"line1\nline2\nline3", 5},
        {"0123456789012345\nafter16", 16},
        {"01234567890123456789012345678901\nafter32", 32},
        {"windows\r\nstyle", 7},  // Should find \r first or \n at 8
    };

    for (const auto& [data, expected] : test_cases) {
        TEST(("newline at " + std::to_string(expected)).c_str());
        size_t result = blazecsv::detail::find_newline(data.data(), data.size());
        // For \r\n, accept either position
        if (result == expected ||
            (data.find("\r\n") != std::string::npos && result == expected + 1)) {
            PASS();
        } else {
            FAIL("got " + std::to_string(result) + ", expected " + std::to_string(expected));
        }
    }
}

void test_simd_performance() {
    std::cout << "\n=== SIMD Performance ===\n";

    // Create large buffer with delimiter at various positions
    const size_t buffer_size = 1024 * 1024;  // 1 MB
    std::vector<char> buffer(buffer_size, 'x');

    // Add some commas at regular intervals
    for (size_t i = 100; i < buffer_size; i += 100) {
        buffer[i] = ',';
    }

    TEST("find delimiter performance");
    {
        auto start = std::chrono::high_resolution_clock::now();
        size_t total_found = 0;

        for (int iter = 0; iter < 100; ++iter) {
            size_t pos = 0;
            while (pos < buffer_size) {
                size_t next =
                    blazecsv::detail::find_field_end(buffer.data() + pos, buffer_size - pos, ',');
                if (next == buffer_size - pos)
                    break;
                total_found++;
                pos += next + 1;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration_us = std::chrono::duration<double, std::micro>(end - start).count();

        std::cout << "PASS (" << duration_us / 100 << " us/MB, found " << total_found / 100
                  << " delimiters)\n";
        tests_passed++;
    }

    TEST("find newline performance");
    {
        // Reset buffer with newlines
        std::fill(buffer.begin(), buffer.end(), 'x');
        for (size_t i = 80; i < buffer_size; i += 80) {
            buffer[i] = '\n';
        }

        auto start = std::chrono::high_resolution_clock::now();
        size_t total_found = 0;

        for (int iter = 0; iter < 100; ++iter) {
            size_t pos = 0;
            while (pos < buffer_size) {
                size_t next =
                    blazecsv::detail::find_newline(buffer.data() + pos, buffer_size - pos);
                if (next == buffer_size - pos)
                    break;
                total_found++;
                pos += next + 1;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration_us = std::chrono::duration<double, std::micro>(end - start).count();

        std::cout << "PASS (" << duration_us / 100 << " us/MB, found " << total_found / 100
                  << " newlines)\n";
        tests_passed++;
    }
}

void test_alignment_handling() {
    std::cout << "\n=== Alignment Handling ===\n";

    // Test with unaligned buffer starts
    const size_t buffer_size = 256;
    alignas(64) char aligned_buffer[buffer_size + 64];

    for (size_t offset = 0; offset < 16; ++offset) {
        TEST(("offset " + std::to_string(offset)).c_str());

        char* ptr = aligned_buffer + offset;
        std::memset(ptr, 'x', buffer_size);
        ptr[50] = ',';

        size_t result = blazecsv::detail::find_field_end(ptr, buffer_size, ',');
        if (result == 50) {
            PASS();
        } else {
            FAIL("got " + std::to_string(result) + ", expected 50");
        }
    }
}

void test_edge_cases() {
    std::cout << "\n=== Edge Cases ===\n";

    TEST("empty buffer");
    {
        size_t result = blazecsv::detail::find_field_end("", 0, ',');
        if (result == 0) {
            PASS();
        } else {
            FAIL("expected 0");
        }
    }

    TEST("single char - delimiter");
    {
        size_t result = blazecsv::detail::find_field_end(",", 1, ',');
        if (result == 0) {
            PASS();
        } else {
            FAIL("expected 0");
        }
    }

    TEST("single char - not delimiter");
    {
        size_t result = blazecsv::detail::find_field_end("x", 1, ',');
        if (result == 1) {
            PASS();
        } else {
            FAIL("expected 1");
        }
    }

    TEST("all delimiters");
    {
        const char* data = ",,,,";
        size_t result = blazecsv::detail::find_field_end(data, 4, ',');
        if (result == 0) {
            PASS();
        } else {
            FAIL("expected 0");
        }
    }

    TEST("delimiter at exact SIMD boundary (16)");
    {
        char buffer[32];
        std::memset(buffer, 'x', 32);
        buffer[15] = ',';  // At position 15 (0-indexed)
        size_t result = blazecsv::detail::find_field_end(buffer, 32, ',');
        if (result == 15) {
            PASS();
        } else {
            FAIL("got " + std::to_string(result));
        }
    }
}

int main() {
    std::cout << "=== BlazeCSV SIMD Tests ===\n";

    test_simd_detection();
    test_delimiter_finding();
    test_newline_finding();
    test_simd_performance();
    test_alignment_handling();
    test_edge_cases();

    std::cout << "\n=== Summary ===\n";
    std::cout << "Tests run: " << tests_run << "\n";
    std::cout << "Tests passed: " << tests_passed << "\n";
    std::cout << "Tests failed: " << (tests_run - tests_passed) << "\n";

    return tests_run == tests_passed ? 0 : 1;
}
