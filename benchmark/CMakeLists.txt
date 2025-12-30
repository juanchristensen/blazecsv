# BlazeCSV Benchmarks

find_package(Threads REQUIRED)

# Compiler-specific optimization flags
if(MSVC)
    set(OPT_FLAGS /O2 /DNDEBUG)
else()
    set(OPT_FLAGS -O3 -DNDEBUG)
endif()

# BlazeCSV benchmark
add_executable(blazecsv_bench blazecsv_bench.cpp)
target_link_libraries(blazecsv_bench PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(blazecsv_bench PRIVATE ${OPT_FLAGS})
