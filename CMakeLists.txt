cmake_minimum_required(VERSION 3.14)
project(TradingSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Include the source directories for your application logic
include_directories(include)

# 2. Add your main core application targets
add_library(trading_engine src/Engine.cpp src/OrderBook.cpp )
add_executable(trading_sim src/main.cpp)
target_link_libraries(trading_sim PRIVATE trading_engine)

# 3. CONFIGURE GOOGLETEST 
# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add the googletest subdirectory so CMake compiles it
add_subdirectory(thirdparty/googletest/googletest-1.14.0)

# 4. Configure and Link the Test Executable
enable_testing()
add_executable(engine_tests tests/test_main.cpp)

# Link against the raw gtest target (bypassing gtest_main)
target_link_libraries(engine_tests PRIVATE 
    trading_engine 
    gtest
)

# Enable testing framework discovery
include(GoogleTest)
gtest_discover_tests(engine_tests DISCOVERY_MODE PRE_TEST)

# For AOS vs SOA experiment
add_executable(aos_soa_bench benchmarks/AosSoaBenchmark.cpp)


