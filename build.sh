#!/bin/bash
if [ ! -f ~/.conan2/profiles/default ]; then
    conan profile detect
fi
mkdir -p build
cd build
conan install .. --output-folder=. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=./conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build .