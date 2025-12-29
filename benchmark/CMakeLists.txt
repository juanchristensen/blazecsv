# BlazeCSV Benchmarks

include(FetchContent)
find_package(Threads REQUIRED)

# Compiler-specific optimization flags
if(MSVC)
    set(OPT_FLAGS /O2 /DNDEBUG)
else()
    set(OPT_FLAGS -O3 -DNDEBUG)
endif()

# Fetch external CSV libraries for comparison
message(STATUS "Fetching CSV libraries for benchmark comparison...")

# vincentlaucsb/csv-parser
FetchContent_Declare(
    csv_parser
    GIT_REPOSITORY https://github.com/vincentlaucsb/csv-parser.git
    GIT_TAG 2.3.0
    GIT_SHALLOW TRUE
)

# p-ranav/csv2
FetchContent_Declare(
    csv2
    GIT_REPOSITORY https://github.com/p-ranav/csv2.git
    GIT_TAG v0.1
    GIT_SHALLOW TRUE
)

# d99kris/rapidcsv
FetchContent_Declare(
    rapidcsv
    GIT_REPOSITORY https://github.com/d99kris/rapidcsv.git
    GIT_TAG v8.84
    GIT_SHALLOW TRUE
)

# ben-strasser/fast-cpp-csv-parser
FetchContent_Declare(
    fast_csv
    GIT_REPOSITORY https://github.com/ben-strasser/fast-cpp-csv-parser.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

# Make libraries available
FetchContent_MakeAvailable(csv_parser csv2 rapidcsv fast_csv)

# Create interface targets for header-only libraries
add_library(csv2_lib INTERFACE)
target_include_directories(csv2_lib INTERFACE ${csv2_SOURCE_DIR}/include)

add_library(rapidcsv_lib INTERFACE)
target_include_directories(rapidcsv_lib INTERFACE ${rapidcsv_SOURCE_DIR}/src)

add_library(fast_csv_lib INTERFACE)
target_include_directories(fast_csv_lib INTERFACE ${fast_csv_SOURCE_DIR})

# Main benchmark
add_executable(csv_benchmark main.cpp)
target_link_libraries(csv_benchmark PRIVATE
    blazecsv::blazecsv
    csv
    csv2_lib
    rapidcsv_lib
    fast_csv_lib
    Threads::Threads
)
target_compile_options(csv_benchmark PRIVATE ${OPT_FLAGS})

# BlazeCSV-only benchmark (no external dependencies)
add_executable(blazecsv_bench blazecsv_bench.cpp)
target_link_libraries(blazecsv_bench PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(blazecsv_bench PRIVATE ${OPT_FLAGS})
