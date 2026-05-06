@echo off
if not exist %USERPROFILE%\.conan2\profiles\default (
    conan profile detect
)
if not exist build mkdir build
cd build
conan install .. --output-folder=. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=./conan_toolchain.cmake
cmake --build .
cd ..