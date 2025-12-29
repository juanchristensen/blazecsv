# BlazeCSV Examples

find_package(Threads REQUIRED)

# Compiler-specific optimization flags
if(MSVC)
    set(OPT_FLAGS /O2)
else()
    set(OPT_FLAGS -O3)
endif()

# Basic usage example
add_executable(basic basic.cpp)
target_link_libraries(basic PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(basic PRIVATE ${OPT_FLAGS})

# Trading data example
add_executable(trading_data trading_data.cpp)
target_link_libraries(trading_data PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(trading_data PRIVATE ${OPT_FLAGS})

# Error handling example
add_executable(error_handling error_handling.cpp)
target_link_libraries(error_handling PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(error_handling PRIVATE ${OPT_FLAGS})

# Parallel parsing example
add_executable(parallel parallel.cpp)
target_link_libraries(parallel PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(parallel PRIVATE ${OPT_FLAGS})
