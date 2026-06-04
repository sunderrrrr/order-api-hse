FROM ubuntu:22.04 AS builder
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake build-essential git python3-pip libpq-dev \
    && pip3 install conan \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN conan profile detect --force

WORKDIR /app

COPY conanfile.txt .
RUN conan install . --output-folder=build --build=missing \
    -s build_type=Release -s compiler.libcxx=libstdc++11

COPY . .
RUN cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG" \
    && cmake --build build --parallel

FROM ubuntu:22.04
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates libpq5 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/order_api /usr/local/bin/order_api
EXPOSE 8080

USER nobody
CMD ["order_api"]