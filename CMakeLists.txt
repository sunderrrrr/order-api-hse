cmake_minimum_required(VERSION 3.15)
project(order_api VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

find_package(oatpp REQUIRED)
find_package(PostgreSQL REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")

add_library(order_lib ${SOURCES})
target_include_directories(order_lib PUBLIC src)

target_link_libraries(order_lib
    oatpp::oatpp
    PostgreSQL::PostgreSQL  
)

add_executable(order_api src/main.cpp)
target_link_libraries(order_api order_lib)

enable_testing()
find_package(doctest REQUIRED)
add_executable(tests test/test_orders.cpp)
target_link_libraries(tests order_lib doctest::doctest)
add_test(NAME OrderTests COMMAND tests)