#include <stdio.h>
#include <getopt.h>

#include "opsplash.h"

char *support[] = {"readinfo", "unpack", "repack"};

void Usage() {
    fprintf(stdout, "Usage:\n"
                    "\topsplash [command] [-i] [-o]\n"
                    "\t\t-i,--input\t\tFile input\n"
                    "\t\t-o,--output\t\tFile/Dir output\n"
                    "\tThis is for oppo splash unpack/repack tool\n"
                    "\tIf you use this just take all risk by your self\n"
                    "Command support below:\n");
    for(int i=0;i<3;i++) {
        fprintf(stdout, "\t%s\n", support[i]);
    }
}

int main(int argc, char* argv[]) {
    //printf("%lld\n", argc);
    //readinfo(argv[1]);
    //unpack(argv[1], "pic");
    //repack(argv[1], "new-splash.img");
    //showup();

    if(argc<2) {
        Usage();
        return 1;
    }

    char *infile, *outfile;
    char *cmd = (char*)malloc(512);
    int exitcode = 0;
    strcpy(cmd, argv[1]);
    infile = (char*)malloc(512);
    outfile = (char*)malloc(512);
    char *optstr = "i:o:";
    struct option opts[] = {
        {"input", 1, NULL, 'i'},
        {"output", 1, NULL, 'o'},
        {0, 0, 0, 0},
    };
    int opt;
    while((opt = getopt_long(argc, argv, optstr, opts, NULL)) != -1){
        switch(opt) {
            case 'i':
                strcpy(infile, optarg);
                break;
            case 'o':
                strcpy(outfile, optarg);
                break;
            case '?':
                if(strchr(optstr, optopt) == NULL){
                    fprintf(stderr, "unknown option '-%c'\n", optopt);
                }else{
                    fprintf(stderr, "option requires an argument '-%c'\n", optopt);
                }
                return 1;
        }
    }
    if(strlen(infile)==0) {
        LOGE("Input file not defined!\n");
        return 1;
    } else if(access(infile, F_OK)!=0) {
        LOGE("Input file not exist!");
        return 1;
    }
    if(strncmp(cmd, "unpack", 6)==0) {
        if(strlen(outfile) == 0) {
            strcpy(outfile, "pic");
            LOGW("Using default outdir : pic\n");
        }
        if(unpack(infile, outfile)!=true) {
            LOGE("Unpack Faild!\n");
            exitcode = 1;
        }
    } else if(strncmp(cmd, "repack", 6)==0) {
        if(strlen(outfile) == 0) {
            strcpy(outfile, "new-splash.img");
            LOGW("Using default out : new-splash.img\n");
        }
        if(repack(infile, outfile)!=true) {
            LOGE("Repack Faild!\n");
            exitcode = 1;
        }
    } else if(strncmp(cmd, "readinfo", 8)==0) {
        if(readinfo(infile)!=true) {
            LOGE("Readinfo Faild!\n");
            exitcode = 1;
        }
    } else {
        LOGE("command not support!\n");
        exitcode = 1;
    }
    free(infile);
    free(outfile);
    free(cmd);
    return exitcode;

}