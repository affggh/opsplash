#ifndef __OPSPLASH_H
#ifdef __cplusplus
extern "C"
{
#endif

// windows binary spec
#ifndef O_BINARY
#define O_BINARY 0
#endif

#include <stdint.h>
#include <iostream>

// Get version info
#include "opsplash_version.h"

#define CHUNK 0x40000
enum
{
    OPSPLASH_GZIP_INFLATE,
    OPSPLASH_GZIP_DEFLATE,
};

#define DEFAULT_SPLASH_IMG "splash.img"
#define DEFAULT_EXTRACT_DIR "pic"
#define OUTPUT_SPLASH_IMG "new-splash.img"
#define DEFAILT_COMPRESS_LEVEL 6

#define DDPH_MAGIC_V1 0x48504444
#define DDPH_HDR_OFFSET 0x0

#pragma pack(push, 1)
typedef struct
{
    uint32_t magic;
    uint32_t flag;
} ddph_hdr;

#define OPPO_SPLASH_MAGIC_V1 "SPLASH LOGO!"
#define OPPO_SPLASH_HDR_OFFSET 0x4000

typedef struct
{
    uint8_t magic[12];
    uint8_t metadata[3][0x40];
    uint8_t zerofill[0x40];
    uint32_t imgnumber;
    uint32_t unknow;
    uint32_t width;
    uint32_t height;
    uint32_t special;
} oppo_splash_hdr;

#define METADATA_OFFSET           \
    (   sizeof(oppo_splash_hdr) + \
        OPPO_SPLASH_HDR_OFFSET)

typedef struct
{
    uint32_t offset;
    uint32_t realsz;
    uint32_t compsz;
    uint8_t name[0x74];
} splash_metadata_hdr;

// Gzip compressed data at offset 0x8000
#define DATA_OFFSET 0x8000

// Define bmp header
// 14 bytes
#define VALID_BMP_MAGIC 0x4d42
#define VALID_BMP_OFFBIT 0x36 // 54 bytes
typedef struct
{
    uint16_t bftype;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfoffBits;
} bmp_hdr;

typedef struct
{
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t imageSize;
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;
    uint32_t colorsImportant;
} bmp_hdr_v3;

#define BMP_HEADER_SIZE (sizeof(bmp_hdr) + sizeof(bmp_hdr_v3))

static inline void banner(void)
{
    std::cout << "# OPSPLASH Version : " << OPSPLASH_MAJOR_VERSION << "." << OPSPLASH_MINOR_VERSION << "." << OPSPLASH_PATCH_VERSION << std::endl;

    std::cout << "# OPSPLASH Author  : " << OPSPLASH_AUTHOR << std::endl;

    std::cout << "# OPPO Splash image modified tool." << std::endl;
}

typedef struct
{
    uint8_t id1;
    uint8_t id2;
    uint8_t cm;
    uint8_t flg;
    uint32_t mtime;
    uint8_t compression;
    uint8_t os;
} gzip_header;

static inline gzip_header get_def_gziphdr(void)
{
    gzip_header hdr{
        .id1 = 0x1f,
        .id2 = 0x8b,
        .cm = 0x8,
        .flg = 0x0,
        .mtime = 0x0,
        .compression = 0,
        .os = 0x3 // linux
    };
    return hdr;
}

typedef struct
{
    uint32_t crc32;
    uint32_t isize;
} gzip_footer;
#pragma pack(pop)
// Define opsplash functions
class opsplash
{
private:
    std::string FilePath;
    std::string OutPath;
    bool ConvertToPng;

    ddph_hdr ddph;
    oppo_splash_hdr splashhdr;
    splash_metadata_hdr metadata[128];

    int fd;

    bool isValidBmp(bmp_hdr *hdr);
    size_t getFileSize(std::string FilePath);
    int parseFile(std::string FilePath);
    void close_all(void);

public:
    void init_cfg(std::string filePath = DEFAULT_SPLASH_IMG,
                  std::string outPath = OUTPUT_SPLASH_IMG,
                  bool conv2png = true);
    void ReadInfo(std::string FilePath = DEFAULT_SPLASH_IMG);
    int Unpack(std::string FilePath = DEFAULT_SPLASH_IMG,
               std::string OutPath = DEFAULT_EXTRACT_DIR);
    int Repack(std::string FilePath = DEFAULT_SPLASH_IMG,
               std::string PicPath = DEFAULT_EXTRACT_DIR,
               std::string OutPath = OUTPUT_SPLASH_IMG);
};
#ifdef __cplusplus
} // extern "C"
#endif
#endif //__OPSPLASH_H
