FROM ubuntu:24.04 AS builder

# Установка зависимостей
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential=12.10ubuntu1 cmake=3.28.3-1build7 \
	cmake-data=3.28.3-1build7 git=1:2.43.0-1ubuntu7.2 \
	git-man=1:2.43.0-1ubuntu7.2 ninja-build=1.11.1-2 \
	zlib1g-dev:amd64=1:1.3.dfsg-3.1ubuntu2.1 ca-certificates=20240203 && \
    rm -rf /var/lib/apt/lists/*

# Клонирование исходников
WORKDIR /opsplash
RUN git clone https://github.com/affggh/opsplash.git . && \
    git config --global --add safe.directory /opsplash && \
    git submodule update --init --recursive

# Патчи
RUN sed -i -E -e 's/O_BINARY\s*\|//g; s/(open\([^,]+,[^)]+)\)/\1, 0644)/' src/libopsplash.cpp

# Сборка
RUN cmake -B build -G Ninja && \
    ninja -C build
FROM scratch AS export
COPY --from=builder /opsplash/build/opsplash /

