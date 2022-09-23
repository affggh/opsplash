#include <stdio.h>
#include "opsplash.h"

bool READSHOWINFO = true;
bool WRITEFAKEHEAD = true;

struct fakehead FAKE;
struct realhead REALHEAD;

size_t gzip(int mode, FILE *fd, const void *buf, size_t size) {
	size_t ret = 0, have, total = 0;
	z_stream strm;
    gz_header head;
	unsigned char out[CHUNK];

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	switch(mode) {
		case 0:
			ret = inflateInit2(&strm, 15 | 16);
			break;
		case 1:
			ret = deflateInit2(&strm, 9, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
			break;
	}
	if (ret != Z_OK)
		fprintf(stderr, "Unable to init zlib stream\n");
	strm.next_in = (void *) buf;
	strm.avail_in = size;

	do {
		strm.avail_out = CHUNK;
		strm.next_out = out;
		switch(mode) {
			case 0:
				ret = inflate(&strm, Z_FINISH);
				break;
			case 1:
				ret = deflate(&strm, Z_FINISH);
				break;
		}
		if (ret == Z_STREAM_ERROR)
			fprintf(stderr, "Error when running gzip\n");
		have = CHUNK - strm.avail_out;
		total += fwrite(out, 1, have, fd);
	} while (strm.avail_out == 0);

	switch(mode) {
		case 0:
			inflateEnd(&strm);
			break;
		case 1:
			deflateEnd(&strm);
			break;
	}
	return total;
}

void LOGE(char *text) {
    fprintf(stderr, "\033[91m[ERROR]: %s\033[0m", text);
}

void LOGW(char *text) {
    fprintf(stderr, "\033[93m[WARNNING]: %s\033[0m", text);
}

void LOGI(char *text) {
    fprintf(stdout, "\033[94m[INFO]: %s\033[0m", text);
}

void LOGS(char *text) {
    fprintf(stdout, "\033[92m[SUCCESS]: %s\033[0m", text);
}

void showup() {
    fprintf(stderr, "\t\t\033[92m[OPPO SPLASH MODIFIED TOOL]\033[0m\n");
    fprintf(stderr, "=---------------------------------------------------------------=\n");
    fprintf(stderr, "\tAUTHOR:\t\033[94m %s \033[0m\tVERSION:\t\033[94m %s \033[0m\n", AUTHOR, VERSION);
    fprintf(stderr, "=---------------------------------------------------------------=\n");
}

// 通过stat结构体 获得文件大小，单位字节
// https://blog.csdn.net/cpp_learner/article/details/123018360
size_t getFileSize1(const char *fileName) {
	if (fileName == NULL) {
		return -1;
	}
	// 这是一个存储文件(夹)信息的结构体，其中有文件大小和创建时间、访问时间、修改时间等
	struct stat statbuf;
	// 提供文件名字符串，获得文件属性结构体
	stat(fileName, &statbuf);
	// 获取文件大小
	size_t filesize = statbuf.st_size;
	return filesize;
}

int readfake(FILE *fp) {
    //struct fakehead HEAD;
    fseek(fp, FAKEOFF, SEEK_SET);
    fread(&FAKE, 1, sizeof(struct fakehead), fp);
    if(memcmp(FAKE.magic, "DDPH", sizeof(FAKE.magic)) != 0) {
        LOGW("Fake header magic read failed, this may not a issue.\n");
        return false;
        };
    if(FAKE.flag != 1) {
        LOGW("Fake header flag read not equal to 1, this may not a issue.\n");
        return false;
    };
    return true;
}

int readreal(FILE *fp) {
    fseek(fp, REALOFF, SEEK_SET);
    fread(&REALHEAD, 1, sizeof(struct realhead), fp);
    LOGI("Checking header magic...\n");
    if(memcmp(REALHEAD.magic, "SPLASH LOGO!", sizeof(REALHEAD.magic)) != 0) {
        LOGE("Failed\n" \
             "Real header magic check failed, this may not a oppo splash image.\n");
        return false;
    } else {
        LOGI("Pass\n");
    }
    return true;
}


int readinfo(char *infile) {
    showup();
    FILE *fp;
    fp = fopen(infile, "rb");
    if(readfake(fp) != true) {
        LOGW("Read fake magic faild, this may not a issue.\n");
        //return false;
    }
    if(readreal(fp) != true) {
        LOGE("Read real header failed, please comfirm this is a oppo splash image!\n");
        return false;
    }
    if(READSHOWINFO == true) {
        fprintf(stdout, "METADATA:\n\tmeta1:%s\n\tmeta2:%s\n\tmeta3:%s\n", 
                            REALHEAD.metadata1, 
                            REALHEAD.metadata2, 
                            REALHEAD.metadata3);
        fprintf(stdout, "Image number:\t%d\n", REALHEAD.imgnumber);
        fprintf(stdout, "Unknow flag:\t%d\n", REALHEAD.unknow);
        fprintf(stdout, "Device width:\t%d\n", REALHEAD.width);
        fprintf(stdout, "Device height:\t%d\n", REALHEAD.height);
        fprintf(stdout, "Compressed flag:\t%d\n", REALHEAD.special);
    }
    struct metadata METADATA[REALHEAD.imgnumber];
    if(READSHOWINFO == true) {
        fprintf(stdout, "NUM\tOFFSET\tREALSZ\tCOMPSZ\tNAME\n");
        fprintf(stdout, "=---------------------------------------------------------------=\n");
    }
    for(int i=0;i<REALHEAD.imgnumber;i++) {
        fread(&METADATA[i], 1, sizeof(struct metadata), fp);
        if(READSHOWINFO == true) {
            fprintf(stdout, "%d\t%d\t%d\t%d\t%s\n", i+1,
                            METADATA[i].offset, 
                            METADATA[i].realsz, 
                            METADATA[i].compsz, 
                            METADATA[i].name);
        }
    }
    if(READSHOWINFO == true) {
        fprintf(stdout, "=---------------------------------------------------------------=\n");
    }
    fclose(fp);
    LOGS("Read Success!\n");
    return true;
}

int unpack(char* infile, char* outdir) {
    showup();
    FILE *fp, *out;
    char *buffer, *filename;
    fprintf(stdout, "File input : \t[%s\n]", infile);
    fprintf(stdout, "Dir output : \t[%s]\n", outdir);
    // Create folder if it not exist 
    if (access(outdir, F_OK)!=0) {
        #ifdef __MINGW32__
        if(mkdir(outdir)!=0) {
        #else
        if(mkdir(outdir, 0777)!=0) {
        #endif
            LOGE("Faild create directory");
            return false;
        }
    }
    fp = fopen(infile, "rb");
    READSHOWINFO = false;  // disable readinfo output
    readinfo(infile);
    fprintf(stdout, "METADATA:\n\tmeta1:%s\n\tmeta2:%s\n\tmeta3:%s\n", 
                        REALHEAD.metadata1, 
                        REALHEAD.metadata2, 
                        REALHEAD.metadata3);
    fprintf(stdout, "Image number:\t%d\n", REALHEAD.imgnumber);
    fprintf(stdout, "Unknow flag:\t%d\n", REALHEAD.unknow);
    fprintf(stdout, "Device width:\t%d\n", REALHEAD.width);
    fprintf(stdout, "Device height:\t%d\n", REALHEAD.height);
    fprintf(stdout, "Compressed flag:\t%d\n", REALHEAD.special);
    LOGI("Extracting...\n");
    struct metadata METADATA[REALHEAD.imgnumber];
    fseek(fp, REALOFF+sizeof(struct realhead), SEEK_SET);
    
    fprintf(stdout, "\tOFFSET\tREALSZ\tCOMPSZ\tNAME\n");
    fprintf(stdout, "=---------------------------------------------------------------=\n");
    for(int i=0;i<REALHEAD.imgnumber;i++) {
        fread(&METADATA[i], 1, sizeof(struct metadata), fp);
    }
    for(int j=0;j<REALHEAD.imgnumber;j++) {
        fprintf(stdout, "Decomp\t%d\t%d\t%d\t%s\n",
                        METADATA[j].offset, 
                        METADATA[j].realsz, 
                        METADATA[j].compsz, 
                        METADATA[j].name);
        buffer = (char*)malloc(METADATA[j].compsz);
        fseek(fp, METADATA[j].offset+DATAOFF, SEEK_SET);
        fread(buffer, METADATA[j].compsz, 1, fp);
        filename = (char*)malloc(512);
        sprintf(filename, "%s/%s%s", outdir, METADATA[j].name, IMGEXT);
        out = fopen(filename, "wb");
        gzip(0, out, buffer, METADATA[j].compsz);
        fclose(out);
        if(getFileSize1(filename)!=METADATA[j].realsz) {
            LOGW("Extract size not same with read.");
        }
        free(filename);
        free(buffer);
    }
    fprintf(stdout, "=---------------------------------------------------------------=\n");
    fclose(fp);
    LOGS("Done!\n");
    return true;
}

bool checkBmp(char *infile) {
    FILE *fp;
    struct bmp_head head;
    if((fp = fopen(infile, "rb"))==NULL) {
        LOGE("Bmp file not exist!\n");
        return false;
    }
    fread(&head, 1, sizeof(struct bmp_head), fp);
    if(head.bftype != 0x4d42) {
        LOGE("Bmp head magic read faild!\n");
        return false;
    }
    if(head.bfoffBits != 0x36) {
        LOGW("Bmp header size not equal to 0x36, image may not show while boot.\n");
        return false;
    }
    if(getFileSize1(infile)!=head.bfSize) {
        LOGE("Bmp header record file size not equal with real size!\n");
        return false;
    }
    fclose(fp);
    return true;
}

int repack(char* infile, char* outfile) {
    showup();
    FILE *fp, *out, *tmp;
    char *buffer, *filename;
    int offset, realsz;
    fprintf(stdout, "File input : \t[%s]\n", infile);
    fprintf(stdout, "File output : \t[%s]\n", outfile);
    fp = fopen(infile, "rb");
    out = fopen(outfile, "wb");
    READSHOWINFO = false;  // disable readinfo output
    readinfo(infile);
    fprintf(stdout, "METADATA:\n\tmeta1:%s\n\tmeta2:%s\n\tmeta3:%s\n", 
                        REALHEAD.metadata1, 
                        REALHEAD.metadata2, 
                        REALHEAD.metadata3);
    fprintf(stdout, "Image number:\t%d\n", REALHEAD.imgnumber);
    fprintf(stdout, "Unknow flag:\t%d\n", REALHEAD.unknow);
    fprintf(stdout, "Device width:\t%d\n", REALHEAD.width);
    fprintf(stdout, "Device height:\t%d\n", REALHEAD.height);
    fprintf(stdout, "Compressed flag:\t%d\n", REALHEAD.special);
    LOGI("Repacking...\n");
    struct metadata METADATA[REALHEAD.imgnumber];
    fseek(fp, REALOFF+sizeof(struct realhead), SEEK_SET);
    if(memcmp(FAKE.magic, "DDPH", sizeof(FAKE.magic))==0) {
        LOGI("Check fake magic pass, write into new splash image.\n");
        fseek(out, FAKEOFF, SEEK_SET);
        fwrite(&FAKE, 1, sizeof(struct fakehead), out);
    } else {
        LOGW("Skip fake magic.\n");
        // return false;
    }
    if(memcmp(REALHEAD.magic, "SPLASH LOGO!", sizeof(REALHEAD.magic))==0) {
        LOGI("Real magic check pass, write into new splash image.\n");
        fseek(out, REALOFF, SEEK_SET);
        fwrite(&REALHEAD, 1, sizeof(struct realhead), out);
    } else {
        LOGE("Real magic check failed, please check your splash image.\n");
        return false;
    }
    offset = 0;
    int infoff;
    fprintf(stdout, "\tOFFSET\tREALSZ\tCOMPSZ\tNAME\n");
    fprintf(stdout, "=---------------------------------------------------------------=\n");
    for(int i=0;i<REALHEAD.imgnumber;i++) {
        fread(&METADATA[i], 1, sizeof(struct metadata), fp);

        filename = (char*)malloc(512);
        sprintf(filename, "pic/%s%s", METADATA[i].name, IMGEXT);

        tmp = fopen(filename, "rb");
        if(tmp == NULL) {
            fprintf(stderr, "%s\n", filename);
            LOGE("Can not open bitmap file!Did you unpack before repack?\n");
            return false;
        }
        if(checkBmp(filename)!=true) {
            LOGE("Bmp file check faild!You picture may not show while booting.\n");
            fprintf(stdout, "\033[93m[!]\033[0m");
        } else {
            fprintf(stdout, "\033[92m[√]\033[0m");
        }
        METADATA[i].offset = offset;
        METADATA[i].realsz = getFileSize1(filename);
        buffer = (char*)malloc(METADATA[i].realsz+1);
        fread(buffer, 1, METADATA[i].realsz, tmp);

        infoff = ftell(out);
        fseek(out, offset+DATAOFF, SEEK_SET);
        METADATA[i].compsz = gzip(1, out, buffer, METADATA[i].realsz);
        fseek(out, infoff, SEEK_SET);
        fwrite(&METADATA[i], 1, sizeof(struct metadata), out);

        //printf("%d\n", METADATA[i].realsz);
        fprintf(stdout, "\t%d\t%d\t%d\t%s\n", 
                        METADATA[i].offset,
                        METADATA[i].realsz,
                        METADATA[i].compsz,
                        METADATA[i].name);
        free(buffer);
        free(filename);
        fclose(tmp);
        offset += METADATA[i].compsz;
    }
    fprintf(stdout, "=---------------------------------------------------------------=\n");
    LOGS("Generate success!\n");
    fclose(fp);
    fclose(out);
    return true;
}

