// BlazeCSV - Parsing Tests
//
// Tests for parsing integers, doubles, booleans, and strings

#include <blazecsv/blazecsv.hpp>

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>

#define TEST(name)                       \
    std::cout << "  " << name << "... "; \
    tests_run++
#define PASS()             \
    std::cout << "PASS\n"; \
    tests_passed++
#define FAIL(msg) std::cout << "FAIL: " << msg << "\n"

static int tests_run = 0;
static int tests_passed = 0;

void test_integer_parsing() {
    std::cout << "\n=== Integer Parsing ===\n";

    const std::string filename = "/tmp/test_int.csv";
    {
        std::ofstream f(filename);
        f << "value\n";
        f << "0\n";
        f << "42\n";
        f << "-123\n";
        f << "2147483647\n";           // INT32_MAX
        f << "-2147483648\n";          // INT32_MIN
        f << "9223372036854775807\n";  // INT64_MAX
    }

    blazecsv::SafeReader<1> reader(filename);
    std::vector<int64_t> values;

    reader.for_each([&values](const auto& fields) {
        values.push_back(fields[0].template value_or<int64_t>(-999));
    });

    TEST("zero");
    if (values[0] == 0) {
        PASS();
    } else {
        FAIL("expected 0");
    }

    TEST("positive");
    if (values[1] == 42) {
        PASS();
    } else {
        FAIL("expected 42");
    }

    TEST("negative");
    if (values[2] == -123) {
        PASS();
    } else {
        FAIL("expected -123");
    }

    TEST("INT32_MAX");
    if (values[3] == 2147483647) {
        PASS();
    } else {
        FAIL("expected 2147483647");
    }

    TEST("INT32_MIN");
    if (values[4] == -2147483648LL) {
        PASS();
    } else {
        FAIL("expected -2147483648");
    }

    TEST("INT64_MAX");
    if (values[5] == 9223372036854775807LL) {
        PASS();
    } else {
        FAIL("expected INT64_MAX");
    }

    std::remove(filename.c_str());
}

void test_double_parsing() {
    std::cout << "\n=== Double Parsing ===\n";

    const std::string filename = "/tmp/test_double.csv";
    {
        std::ofstream f(filename);
        f << "value\n";
        f << "0.0\n";
        f << "3.14159\n";
        f << "-2.71828\n";
        f << "1.23e10\n";
        f << "1.23e-10\n";
        f << "1e308\n";
        f << ".5\n";
        f << "-.5\n";
    }

    blazecsv::SafeReader<1> reader(filename);
    std::vector<double> values;

    reader.for_each([&values](const auto& fields) { values.push_back(fields[0].value_or(0.0)); });

    auto approx_eq = [](double a, double b, double eps = 1e-6) {
        return std::abs(a - b) < eps || std::abs(a - b) / std::max(std::abs(a), std::abs(b)) < eps;
    };

    TEST("zero");
    if (approx_eq(values[0], 0.0)) {
        PASS();
    } else {
        FAIL("expected 0.0");
    }

    TEST("pi");
    if (approx_eq(values[1], 3.14159)) {
        PASS();
    } else {
        FAIL("expected 3.14159");
    }

    TEST("negative e");
    if (approx_eq(values[2], -2.71828)) {
        PASS();
    } else {
        FAIL("expected -2.71828");
    }

    TEST("scientific positive");
    if (approx_eq(values[3], 1.23e10)) {
        PASS();
    } else {
        FAIL("expected 1.23e10");
    }

    TEST("scientific negative");
    if (approx_eq(values[4], 1.23e-10)) {
        PASS();
    } else {
        FAIL("expected 1.23e-10");
    }

    TEST("large exponent");
    if (values[5] > 1e307) {
        PASS();
    } else {
        FAIL("expected ~1e308");
    }

    TEST("leading decimal");
    if (approx_eq(values[6], 0.5)) {
        PASS();
    } else {
        FAIL("expected 0.5");
    }

    TEST("negative leading decimal");
    if (approx_eq(values[7], -0.5)) {
        PASS();
    } else {
        FAIL("expected -0.5");
    }

    std::remove(filename.c_str());
}

void test_boolean_parsing() {
    std::cout << "\n=== Boolean Parsing ===\n";

    const std::string filename = "/tmp/test_bool.csv";
    {
        std::ofstream f(filename);
        f << "value\n";
        f << "true\n";
        f << "false\n";
        f << "True\n";
        f << "False\n";
        f << "TRUE\n";
        f << "FALSE\n";
        f << "1\n";
        f << "0\n";
        f << "yes\n";
        f << "no\n";
    }

    blazecsv::SafeReader<1> reader(filename);
    std::vector<std::optional<bool>> values;

    reader.for_each([&values](const auto& fields) {
        values.push_back(fields[0].template as_optional<bool>());
    });

    TEST("true lowercase");
    if (values[0] && *values[0] == true) {
        PASS();
    } else {
        FAIL("expected true");
    }

    TEST("false lowercase");
    if (values[1] && *values[1] == false) {
        PASS();
    } else {
        FAIL("expected false");
    }

    TEST("True mixed");
    if (values[2] && *values[2] == true) {
        PASS();
    } else {
        FAIL("expected true");
    }

    TEST("False mixed");
    if (values[3] && *values[3] == false) {
        PASS();
    } else {
        FAIL("expected false");
    }

    TEST("TRUE uppercase");
    if (values[4] && *values[4] == true) {
        PASS();
    } else {
        FAIL("expected true");
    }

    TEST("FALSE uppercase");
    if (values[5] && *values[5] == false) {
        PASS();
    } else {
        FAIL("expected false");
    }

    TEST("1 as true");
    if (values[6] && *values[6] == true) {
        PASS();
    } else {
        FAIL("expected true");
    }

    TEST("0 as false");
    if (values[7] && *values[7] == false) {
        PASS();
    } else {
        FAIL("expected false");
    }

    TEST("yes as true");
    if (values[8] && *values[8] == true) {
        PASS();
    } else {
        FAIL("expected true");
    }

    TEST("no as false");
    if (values[9] && *values[9] == false) {
        PASS();
    } else {
        FAIL("expected false");
    }

    std::remove(filename.c_str());
}

void test_string_parsing() {
    std::cout << "\n=== String Parsing ===\n";

    const std::string filename = "/tmp/test_string.csv";
    {
        std::ofstream f(filename);
        f << "name,description\n";
        f << "Alice,Hello World\n";
        f << "Bob,\n";
        f << "Charlie,\"Quoted, with comma\"\n";
        f << "Diana,\"Line1\nLine2\"\n";
    }

    blazecsv::TurboReader<2> reader(filename);
    std::vector<std::pair<std::string, std::string>> values;

    reader.for_each([&values](const auto& fields) {
        values.emplace_back(std::string(fields[0].view()), std::string(fields[1].view()));
    });

    TEST("simple string");
    if (values[0].first == "Alice" && values[0].second == "Hello World") {
        PASS();
    } else {
        FAIL("expected Alice/Hello World");
    }

    TEST("empty string");
    if (values[1].first == "Bob" && values[1].second.empty()) {
        PASS();
    } else {
        FAIL("expected Bob/empty");
    }

    std::remove(filename.c_str());
}

void test_tsv_parsing() {
    std::cout << "\n=== TSV Parsing ===\n";

    const std::string filename = "/tmp/test_tsv.tsv";
    {
        std::ofstream f(filename);
        f << "id\tname\tvalue\n";
        f << "1\tAlice\t100\n";
        f << "2\tBob\t200\n";
    }

    blazecsv::TsvTurboReader<3> reader(filename);
    std::vector<int> ids;

    reader.for_each([&ids](const auto& fields) { ids.push_back(fields[0].value_or(-1)); });

    TEST("tsv row 1");
    if (ids[0] == 1) {
        PASS();
    } else {
        FAIL("expected 1");
    }

    TEST("tsv row 2");
    if (ids[1] == 2) {
        PASS();
    } else {
        FAIL("expected 2");
    }

    std::remove(filename.c_str());
}

void test_header_access() {
    std::cout << "\n=== Header Access ===\n";

    const std::string filename = "/tmp/test_header.csv";
    {
        std::ofstream f(filename);
        f << "id,name,score\n";
        f << "1,Alice,95\n";
    }

    blazecsv::TurboReader<3> reader(filename);
    auto headers = reader.headers();

    TEST("header count");
    if (headers.size() == 3) {
        PASS();
    } else {
        FAIL("expected 3 headers");
    }

    TEST("header names");
    if (headers[0] == "id" && headers[1] == "name" && headers[2] == "score") {
        PASS();
    } else {
        FAIL("headers don't match");
    }

    std::remove(filename.c_str());
}

int main() {
    std::cout << "=== BlazeCSV Parsing Tests ===\n";

    test_integer_parsing();
    test_double_parsing();
    test_boolean_parsing();
    test_string_parsing();
    test_tsv_parsing();
    test_header_access();

    std::cout << "\n=== Summary ===\n";
    std::cout << "Tests run: " << tests_run << "\n";
    std::cout << "Tests passed: " << tests_passed << "\n";
    std::cout << "Tests failed: " << (tests_run - tests_passed) << "\n";

    return tests_run == tests_passed ? 0 : 1;
}
