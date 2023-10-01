# OPSPLASH

oppo splash.img modified tool.

# Usage

## ***unpack***
```sh
./opsplash -i splash.img -d
# if you wanna extract file convert to png format
./opsplash -i splash.img -d -c
# if you wanna defined custom output dir or output file
./opsplash -i splash.img -d -o pic -c
```

## ***repack***
```sh
./opsplash -i splash.img
# if you pic in different dir
./opsplash -i splash.img -o pic
# if you pic dir include no bmp but all png pics
./opsplash -i splash.img -c -o pic
```

# Build
```sh
# I use cmake x ninja to build
cmake -B build -G Ninja
ninja -C build

# you can use makefile to build
cmake -B build
make -C build -j$(nproc --all)
```

# Thanks
***[lodepng](https://lodev.org/lodepng)***
`lodepng is a powerful library to encode or decode png pic, it also provide deflate/inflate gzip function and crc32 check sum function.`
