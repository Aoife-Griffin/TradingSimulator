cmake_minimum_required(VERSION 3.24)
project(TradingSimulator VERSION 1.0.0 LANGUAGES CXX)

### Force C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

### Compiler warnings 
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

### Define core trading engine library
add_library(trading_engine 
    src/Engine.cpp
)

target_include_directories(trading_engine PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

### Define main executable
add_executable(trading_sim src/main.cpp)
target_link_libraries(trading_sim PRIVATE trading_engine)

### Test setup
enable_testing()

### Using thirdparty folder instead of downloading off internet
add_subdirectory(thirdparty/googletest/googletest-1.14.0)

### Define test executable
add_executable(engine_tests tests/test_main.cpp)
target_link_libraries(engine_tests PRIVATE trading_engine gtest_main)

### Run tests
include(GoogleTest)
gtest_discover_tests(engine_tests)
