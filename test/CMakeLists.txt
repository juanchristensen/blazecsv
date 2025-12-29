# BlazeCSV Tests

find_package(Threads REQUIRED)

# Compiler-specific optimization flags
if(MSVC)
    set(OPT_FLAGS /O2)
else()
    set(OPT_FLAGS -O2)
endif()

# Test parsing functionality
add_executable(test_parsing test_parsing.cpp)
target_link_libraries(test_parsing PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(test_parsing PRIVATE ${OPT_FLAGS})
add_test(NAME test_parsing COMMAND test_parsing)

# Test SIMD functionality
add_executable(test_simd test_simd.cpp)
target_link_libraries(test_simd PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(test_simd PRIVATE ${OPT_FLAGS})
add_test(NAME test_simd COMMAND test_simd)

# Test error handling
add_executable(test_errors test_errors.cpp)
target_link_libraries(test_errors PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(test_errors PRIVATE ${OPT_FLAGS})
add_test(NAME test_errors COMMAND test_errors)

# Comprehensive tests (edge cases, dates, line endings, parallel correctness)
add_executable(test_comprehensive test_comprehensive.cpp)
target_link_libraries(test_comprehensive PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(test_comprehensive PRIVATE ${OPT_FLAGS})
add_test(NAME test_comprehensive COMMAND test_comprehensive)
