cmake_minimum_required(VERSION 3.15)
project(order_api VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ВАЖНО: Указываем, что ищем конфигурационные файлы (от Conan)
# Это заставит CMake искать PostgreSQLConfig.cmake вместо FindPostgreSQL.cmake
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# Conan подключает зависимости
find_package(oatpp REQUIRED)
find_package(PostgreSQL REQUIRED)

# Основные исходники (без main.cpp — удобно для тестов)
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")

add_library(order_lib ${SOURCES})
target_include_directories(order_lib PUBLIC src)

# Исправлено: PostgreSQL::PostgreSQL может не существовать, используем PostgreSQL::libpq
# Или просто добавим пути вручную
target_link_libraries(order_lib
    oatpp::oatpp
    PostgreSQL::PostgreSQL  # вместо PostgreSQL::PostgreSQL
)

# Основной исполняемый файл
add_executable(order_api src/main.cpp)
target_link_libraries(order_api order_lib)

# Тесты
enable_testing()
find_package(doctest REQUIRED)
add_executable(tests test/test_orders.cpp)
target_link_libraries(tests order_lib doctest::doctest)
add_test(NAME OrderTests COMMAND tests)