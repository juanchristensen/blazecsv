# BlazeCSV Examples

find_package(Threads REQUIRED)

# Basic usage example
add_executable(basic basic.cpp)
target_link_libraries(basic PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(basic PRIVATE -O3)

# Trading data example
add_executable(trading_data trading_data.cpp)
target_link_libraries(trading_data PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(trading_data PRIVATE -O3)

# Error handling example
add_executable(error_handling error_handling.cpp)
target_link_libraries(error_handling PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(error_handling PRIVATE -O3)

# Parallel parsing example
add_executable(parallel parallel.cpp)
target_link_libraries(parallel PRIVATE blazecsv::blazecsv Threads::Threads)
target_compile_options(parallel PRIVATE -O3)
