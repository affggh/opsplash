#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
// support gzip decompress and compress
#include <zlib.h>
#if (defined __CYGWIN__) || (defined __linux__)
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#elif (defined __MINGW32__)
#include <io.h>
#endif

/* CHUNK is the size of the memory chunk used by the zlib routines. */
#ifndef __OPSPLASH
#define __OPSPLASH
#define CHUNK 0x40000
size_t gzip(int mode, FILE *fd, const void *buf, size_t size);
// Source from magiskboot v17.1 
// https://github.com/alitekin2fx/magiskboot/blob/master/compress.c
// Mode: 0 = decode; 1 = encode

#define AUTHOR "affggh"
#define VERSION "1.1"

#define IMGEXT ".bmp"

#define FAKEOFF 0
struct fakehead {
    char magic[4];
    int flag;
};

#define REALOFF 0x4000
struct realhead {
    char magic[12];
    char metadata1[0x40];
    char metadata2[0x40];
    char metadata3[0x40];
    char zerofill[0x40];
    int imgnumber;
    int unknow;
    int width;
    int height;
    int special;
};

// Thie rely on REALHEAD.imgnumber
// repack need to read this
#define METAOFF REALOFF+sizeof(struct realhead)
struct metadata {
    int offset;
    int realsz;
    int compsz;
    char name[0x74];
};

// This is all gzip padding data
#define DATAOFF 0x8000

// Bmp head for check
struct bmp_head {
unsigned short bftype;
unsigned int bfSize;
unsigned short bfReserved1;
unsigned short bfReserved2;
unsigned int bfoffBits;//54
}__attribute__((packed)); //14byte

// Define show more color infos
void LOGE(char *text);
void LOGW(char *text);
void LOGI(char *text);
void LOGS(char *text);

void showup();
char diraddfile(char *dir, char *file);
size_t getFileSize1(const char *fileName);

// Define functions
int readinfo(char *infile);
int readfake(FILE *fp);
int readreal(FILE *fp);
int unpack(char* infile, char* outdir);
bool checkBmp(char *infile);
int repack(char* infile, char* outfile);
#endif // __OPSPLASH