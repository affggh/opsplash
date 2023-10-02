cmake_minimum_required(VERSION 3.20)

project(libopsplash 
    LANGUAGES C CXX 
    VERSION 1.14.514)

set(CMAKE_CXX_STANDARD 17)

set(libopsplash_AUTHOR "\"affggh\"")

set(LIB_DIR ".")

include(cmake/lodepng.cmake)

include_directories(
    "include"
    "zlib"
    "lodepng"
    "${CMAKE_BINARY_DIR}"
)

configure_file(include/opsplash_version.h.in opsplash_version.h)

set(OPSPLASH_INCLUDES "include/opsplash.h")

set(OPSPLASH_SRCS
    "src/libopsplash.cpp"
)

add_library(libopsplash STATIC ${OPSPLASH_SRCS})
target_compile_options(libopsplash PUBLIC
    "-O3"
    "-Wall"
    "-Wformat"
    "-Wextra"
)

add_executable(opsplash "src/opsplash.cpp")
target_link_options(opsplash PUBLIC
    "-static" "-s"
)
target_link_libraries(opsplash libopsplash lodepng)
