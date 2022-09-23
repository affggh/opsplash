CC = gcc
STRIP = strip

override CFLAGS := $(CFLAGS) -O3 -Wno-unused-result
override STRIPFLAGS := --strip-all
override LDFLAGS := $(LDFLAGS) -lz -static

INCLUDE = -Iinclude

OPSPLASH_OBJ = src/opsplash.o src/main.o

%.o: %.c
#@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) $(INCLUDE) -c $^ -o $@

opsplash: $(OPSPLASH_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	$(STRIP) $(STRIPFLAGS) $@

clean:
	rm -f src/*.o opsplash