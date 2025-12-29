# BlazeCSV - High-Performance CSV Parser
# https://github.com/yourusername/blazecsv
#
# MIT License

cmake_minimum_required(VERSION 3.14)
project(blazecsv
    VERSION 1.0.0
    DESCRIPTION "Blazing fast, header-only CSV parser for C++23"
    HOMEPAGE_URL "https://github.com/yourusername/blazecsv"
    LANGUAGES CXX
)

# =============================================================================
# OPTIONS
# =============================================================================

option(BLAZECSV_BUILD_EXAMPLES "Build examples" OFF)
option(BLAZECSV_BUILD_TESTS "Build tests" OFF)
option(BLAZECSV_BUILD_BENCHMARKS "Build benchmarks" OFF)

# =============================================================================
# HEADER-ONLY LIBRARY
# =============================================================================

add_library(blazecsv INTERFACE)
add_library(blazecsv::blazecsv ALIAS blazecsv)

target_include_directories(blazecsv INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(blazecsv INTERFACE cxx_std_23)

# Platform-specific optimizations
if(APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    target_compile_options(blazecsv INTERFACE
        $<$<COMPILE_LANGUAGE:CXX>:-mcpu=apple-m1>
    )
endif()

# =============================================================================
# SUBDIRECTORIES
# =============================================================================

if(BLAZECSV_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(BLAZECSV_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(BLAZECSV_BUILD_BENCHMARKS)
    add_subdirectory(benchmark)
endif()

# =============================================================================
# INSTALL
# =============================================================================

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS blazecsv
    EXPORT blazecsv-targets
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT blazecsv-targets
    FILE BlazeCSVConfig.cmake
    NAMESPACE blazecsv::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/blazecsv
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/BlazeCSVConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/BlazeCSVConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/blazecsv
)
