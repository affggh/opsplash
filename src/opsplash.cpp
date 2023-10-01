#include <iostream>
#include <filesystem>
#include <cstring>
#include <getopt.h>

#include "opsplash.h"

struct opsplash_cfg {
    char *input;
    char *output;
    char *picdir;
    int convert2png; // repack or unpack use bmp directly, no convert png.
    int unpack;
} cfg;

using namespace std;

static void usage(const char *progname) {
    banner();
    std::filesystem::path progpath = progname;
    cout << progpath.filename() << " -i <splash.img> [-o] <pic|new-splash.img> [-c] [-d] [-p] <pic>" << endl <<
        "\t-i,--input      File input." << endl <<
        "\t-o,--output     File or Dir output, if you unpack defined output, then repack must defined it too." << endl <<
        "\t-c,--convert    Convert output pic format into png, if you unpack with -c repack must with -c." << endl <<
        "\t-d,--decompress Switch mode to unpack, default is repack." << endl <<
        "\t-p,--picdir     Use custom pic dir when repack splash image." << endl << endl <<
        "Exapmle : " << progpath.filename() << " -i splash.img -o pic -d -c" << endl <<
        "          " << progpath.filename() << " -i splash.img -o new-splash.img -c -p pic" << endl << endl <<
        "Copyright by " << OPSPLASH_AUTHOR << "." << endl;
}

static void parse_opts(int argc, char** argv) {
    int option;
    cfg.picdir = nullptr;
    cfg.input = nullptr;
    cfg.convert2png = false;
    cfg.unpack = false;
    cfg.output = nullptr;
    
    static struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"input", required_argument, nullptr, 'i'},
        {"output", optional_argument, nullptr, 'o'},
        {"convert", optional_argument, &cfg.convert2png, 1},
        {"decompress", optional_argument, &cfg.unpack, 1},
        {"picdir", optional_argument, nullptr, 'p'},
        {nullptr, 0, nullptr, 0}
    };

    while ((option = getopt_long(argc, argv, "hi:o:cdp:", long_options, nullptr)) != -1) {
        switch (option) {
            case 'h':
                usage(argv[0]);
                exit(0);
                break;
            case 'i':
                cfg.input = strdup(optarg);
                break;
            case 'o':
                cfg.output = strdup(optarg);
                break;
            case 'c':
                cout << "# Enable png convert." << endl;
                cfg.convert2png = 1;
                break;
            case 'd':
                cout << "# Unpack oppo splash." << endl;
                cfg.unpack = 1;
                break;
            case 'p':
                cout << "# Use custom pic dir : " << optarg << endl;
                cfg.picdir = strdup(optarg);
                break;
            default:
                // 处理其他选项
                break;
        }
    }
}

int main(int argc, char** argv) {
    opsplash s;
    int ret = 0;
    
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    parse_opts(argc, argv);
    s.init_cfg(DEFAULT_SPLASH_IMG, ((cfg.unpack) ? DEFAULT_EXTRACT_DIR : OUTPUT_SPLASH_IMG), (cfg.convert2png) ? true : false);

    if (cfg.unpack) {
        ret = s.Unpack(cfg.input, (cfg.output == nullptr) ? DEFAULT_EXTRACT_DIR : cfg.output);
    } else {
        ret = s.Repack(cfg.input, 
                       (cfg.picdir == nullptr) ? DEFAULT_EXTRACT_DIR : cfg.picdir,
                       (cfg.output == nullptr) ? OUTPUT_SPLASH_IMG : cfg.output);
    }
    return ret;

}