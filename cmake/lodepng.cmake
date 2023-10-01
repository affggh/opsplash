set(TARGET lodepng)

set(TARGET_SRC_DIR "${LIB_DIR}/lodepng")

set(TARGET_CFLAGS
    "-O3"
    "-Wall"
    "-Werror"
    "-Wno-unused"
    "-Wno-unused-parameter"
)

set(lodepng_srcs
    "${LIB_DIR}/lodepng/lodepng.cpp"
)

add_library(${TARGET} STATIC ${lodepng_srcs})

target_include_directories(${TARGET} PRIVATE ${libz_headers})

target_compile_options(${TARGET} PRIVATE
	"$<$<COMPILE_LANGUAGE:C>:${TARGET_CFLAGS}>"
	"$<$<COMPILE_LANGUAGE:CXX>:${TARGET_CFLAGS}>"
)