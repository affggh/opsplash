#include <iostream>
#include <iomanip>
#include <filesystem>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "opsplash.h"
#include "lodepng.h"

using namespace std;
namespace fs = filesystem;

void opsplash::init_cfg(string filePath, string outPath, bool conv2png) {
    FilePath = filePath;
    OutPath = outPath;
	ConvertToPng = conv2png;
}

bool opsplash::isValidBmp(bmp_hdr *hdr) {
    if (hdr->bftype == VALID_BMP_MAGIC && hdr->bfoffBits == VALID_BMP_OFFBIT) {
        return true;
    }
    return false;
}

size_t opsplash::getFileSize(string FilePath) {
    struct stat st;

    stat(FilePath.c_str(), &st);
    return st.st_size;
}

static void readDDPH(int fd, ddph_hdr *hdr) {
	lseek(fd, DDPH_HDR_OFFSET, SEEK_SET);
    read(fd, hdr, sizeof(ddph_hdr));
}

static void readSPLASH(int fd, oppo_splash_hdr *hdr) {
    lseek(fd, OPPO_SPLASH_HDR_OFFSET, SEEK_SET);
    read(fd, hdr, sizeof(oppo_splash_hdr));
}

static void parseMETADATA(int fd, oppo_splash_hdr *hdr, splash_metadata_hdr *metadata) {
	off_t offset = OPPO_SPLASH_HDR_OFFSET+sizeof(oppo_splash_hdr);
	splash_metadata_hdr buf;
	for (uint32_t i=0; i<hdr->imgnumber; i++) {
		lseek(fd, offset, SEEK_SET);
		read(fd, &buf, sizeof(splash_metadata_hdr));
		memcpy(&metadata[i], &buf, sizeof(buf));
		offset += sizeof(splash_metadata_hdr);
	}
}

void opsplash::close_all(void) {
	close(fd);
}

int opsplash::parseFile(string FilePath) {
	int ret = 0;
	auto fileSize = opsplash::getFileSize(FilePath);
	fd = open(FilePath.c_str(), O_BINARY | O_RDONLY | O_EXCL);

	if (!fs::exists(FilePath)) {
		cerr << "Error : File may nost exist." << endl;
		ret = EEXIST;
		goto exitparseFile;
	}

	if (fileSize < DATA_OFFSET) {
		cerr << "Error : File size is too small." << endl;
		ret = EEXIST;
		goto exitparseFile;
	}

	if (!fd) {
		cerr << "Error : Cannot open file [" << FilePath << "]." << endl;
		ret = EIO;
		goto exitparseFile;
	}

	readDDPH(fd, &ddph);
	if (ddph.magic == DDPH_MAGIC_V1) {
		cout << "# Detect DDPH header, we still dont know what is." << endl;
		cout << "# So we just keep it to avoid some issue." << endl;
		cout << "# DDPH may have flag :" << hex << ddph.flag << endl;
	}
	readSPLASH(fd, &splashhdr);
	if (strncmp((const char*)splashhdr.magic, OPPO_SPLASH_MAGIC_V1, sizeof(splashhdr.magic))) {
		cout << "Error : File is not a valid oppo splash image." << endl;
		ret = EINVAL;
		goto exitparseFile;
	}
	if (splashhdr.imgnumber == 0) {
		cerr << "Error : This splash image include zero pic." << endl;
		ret = ENODATA;
		goto exitparseFile;
	}
	
	parseMETADATA(fd, &splashhdr, metadata);

	return ret;

exitparseFile:
	close(fd);
	return ret;

}

void opsplash::ReadInfo(string FilePath) {
	int ret = parseFile(FilePath);

	banner();

	if (ret) {
		cerr << "Error : Cannot parse file [" << FilePath << "]." << endl;
		return;
	}

	fputs("# Magic        : ", stdout);
	for (uint64_t i = 0; i<sizeof(splashhdr.magic); i++) {
		fputc(splashhdr.magic[i], stdout);
	}
	fputc('\n', stdout);
	cout <<
			"# METADATA[0]  : " << splashhdr.metadata[0] << endl <<
			"# METADATA[1]  : " << splashhdr.metadata[1] << endl <<
			"# METADATA[2]  : " << splashhdr.metadata[2] << endl <<
			"# Image Number : " << dec << splashhdr.imgnumber << endl <<
			"# Unknow flag  : " << hex << setw(8) << setfill('0') << splashhdr.unknow << endl <<
			"# Width        : " << dec << splashhdr.width << endl <<
			"# Height       : " << dec << splashhdr.height << endl <<
			"# Special flag : " << hex << setw(8) << setfill('0') << splashhdr.special << endl;
	
	cout << "# Picture info :" << endl <<
			"#\tOFFSET\t\tREALSZ\t\tCOMPSZ\t\tNAME" << endl;
	for (uint32_t i=0; i<splashhdr.imgnumber; i++) {
		cout << "#\t" << uppercase << hex << setw(8) << setfill('0') << metadata[i].offset << "\t" <<
				uppercase << hex << setw(8) << setfill('0') << metadata[i].realsz << "\t" <<
				uppercase << hex << setw(8) << setfill('0') << metadata[i].compsz << "\t" <<
				metadata[i].name << endl;
	}
}

// function below from https://github.com/lvandeve/lodepng/blob/master/examples

//returns 0 if all went ok, non-0 if error
//output image is always given in RGBA (with alpha channel), even if it's a BMP without alpha channel
static unsigned decodeBMP(std::vector<unsigned char>& image, unsigned& w, unsigned& h, const std::vector<unsigned char>& bmp) {
  static const unsigned MINHEADER = 54; //minimum BMP header size

  if(bmp.size() < MINHEADER) return -1;
  if(bmp[0] != 'B' || bmp[1] != 'M') return 1; //It's not a BMP file if it doesn't start with marker 'BM'
  unsigned pixeloffset = bmp[10] + 256 * bmp[11]; //where the pixel data starts
  //read width and height from BMP header
  w = bmp[18] + bmp[19] * 256;
  h = bmp[22] + bmp[23] * 256;
  //read number of channels from BMP header
  if(bmp[28] != 24 && bmp[28] != 32) return 2; //only 24-bit and 32-bit BMPs are supported.
  unsigned numChannels = bmp[28] / 8;

  //The amount of scanline bytes is width of image times channels, with extra bytes added if needed
  //to make it a multiple of 4 bytes.
  unsigned scanlineBytes = w * numChannels;
  if(scanlineBytes % 4 != 0) scanlineBytes = (scanlineBytes / 4) * 4 + 4;

  unsigned dataSize = scanlineBytes * h;
  if(bmp.size() < dataSize + pixeloffset) return 3; //BMP file too small to contain all pixels

  image.resize(w * h * 4);

  /*
  There are 3 differences between BMP and the raw image buffer for LodePNG:
  -it's upside down
  -it's in BGR instead of RGB format (or BRGA instead of RGBA)
  -each scanline has padding bytes to make it a multiple of 4 if needed
  The 2D for loop below does all these 3 conversions at once.
  */
  for(unsigned y = 0; y < h; y++)
  for(unsigned x = 0; x < w; x++) {
    //pixel start byte position in the BMP
    unsigned bmpos = pixeloffset + (h - y - 1) * scanlineBytes + numChannels * x;
    //pixel start byte position in the new raw image
    unsigned newpos = 4 * y * w + 4 * x;
    if(numChannels == 3) {
      image[newpos + 0] = bmp[bmpos + 2]; //R
      image[newpos + 1] = bmp[bmpos + 1]; //G
      image[newpos + 2] = bmp[bmpos + 0]; //B
      image[newpos + 3] = 255;            //A
    } else {
      image[newpos + 0] = bmp[bmpos + 2]; //R
      image[newpos + 1] = bmp[bmpos + 1]; //G
      image[newpos + 2] = bmp[bmpos + 0]; //B
      image[newpos + 3] = bmp[bmpos + 3]; //A
    }
  }
  return 0;
}

//Input image must be RGB buffer (3 bytes per pixel), but you can easily make it
//support RGBA input and output by changing the inputChannels and/or outputChannels
//in the function to 4.
static void encodeBMP(std::vector<unsigned char>& bmp, const unsigned char* image, int w, int h) {
  //3 bytes per pixel used for both input and output.
  int inputChannels = 3;
  int outputChannels = 3;

  //bytes 0-13
  bmp.push_back('B'); bmp.push_back('M'); //0: bfType
  bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); //2: bfSize; size not yet known for now, filled in later.
  bmp.push_back(0); bmp.push_back(0); //6: bfReserved1
  bmp.push_back(0); bmp.push_back(0); //8: bfReserved2
  bmp.push_back(54 % 256); bmp.push_back(54 / 256); bmp.push_back(0); bmp.push_back(0); //10: bfOffBits (54 header bytes)

  //bytes 14-53
  bmp.push_back(40); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //14: biSize
  bmp.push_back(w % 256); bmp.push_back(w / 256); bmp.push_back(0); bmp.push_back(0); //18: biWidth
  bmp.push_back(h % 256); bmp.push_back(h / 256); bmp.push_back(0); bmp.push_back(0); //22: biHeight
  bmp.push_back(1); bmp.push_back(0); //26: biPlanes
  bmp.push_back(outputChannels * 8); bmp.push_back(0); //28: biBitCount
  bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //30: biCompression
  bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //34: biSizeImage
  bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //38: biXPelsPerMeter
  bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //42: biYPelsPerMeter
  bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //46: biClrUsed
  bmp.push_back(0); bmp.push_back(0); bmp.push_back(0); bmp.push_back(0);  //50: biClrImportant

  /*
  Convert the input RGBRGBRGB pixel buffer to the BMP pixel buffer format. There are 3 differences with the input buffer:
  -BMP stores the rows inversed, from bottom to top
  -BMP stores the color channels in BGR instead of RGB order
  -BMP requires each row to have a multiple of 4 bytes, so sometimes padding bytes are added between rows
  */

  int imagerowbytes = outputChannels * w;
  imagerowbytes = imagerowbytes % 4 == 0 ? imagerowbytes : imagerowbytes + (4 - imagerowbytes % 4); //must be multiple of 4

  for(int y = h - 1; y >= 0; y--) { //the rows are stored inversed in bmp
    int c = 0;
    for(int x = 0; x < imagerowbytes; x++) {
      if(x < w * outputChannels) {
        int inc = c;
        //Convert RGB(A) into BGR(A)
        if(c == 0) inc = 2;
        else if(c == 2) inc = 0;
        bmp.push_back(image[inputChannels * (w * y + x / outputChannels) + inc]);
      }
      else bmp.push_back(0);
      c++;
      if(c >= outputChannels) c = 0;
    }
  }

  // Fill in the size
  bmp[2] = bmp.size() % 256;
  bmp[3] = (bmp.size() / 256) % 256;
  bmp[4] = (bmp.size() / 65536) % 256;
  bmp[5] = bmp.size() / 16777216;
}

int opsplash::Unpack(string FilePath, string OutPath) {
	banner();
	auto ret = parseFile(FilePath);

	// Treat out path as out dir
	fs::path OutDir = OutPath;
	fs::path OutFile;
	string FileName;
	off_t offset;
	uint8_t *outbuf, *inbuf;
	size_t outsize, insize;
	bmp_hdr *bmphdr;
	bmp_hdr_v3 *bmpinfo;
	vector<uint8_t> bmpdata, pngdata, imagedata;

	if (ret) {
		cerr << "Error : Cannot parse file [" << FilePath << "]." << endl;
		return ret;
	}

	if (!fs::exists(OutDir)) {
		cout << "# Seems outdir " << OutDir << " does not exist, creating..." << endl;
		fs::create_directories(OutDir);
	}

	for (uint32_t i=0; i<splashhdr.imgnumber; i++) {
		OutFile = OutDir;
		FileName = (char *)metadata[i].name; ConvertToPng ? FileName.append(".png") : FileName.append(".bmp");
		OutFile.append(FileName);

		cout << "# Extract : " << OutFile << " ..." << endl;

		// Real offset equals DATA_OFFET + metadata.offset
		offset = DATA_OFFSET + metadata[i].offset;
		lseek(fd, offset, SEEK_SET);

		outbuf = (uint8_t *)malloc(metadata[i].realsz);
		inbuf = (uint8_t *)malloc(metadata[i].compsz);

		read(fd, inbuf, metadata[i].compsz);

		outsize = 0;
		insize = metadata[i].compsz - 10 - 8; // no header and crc32, isize
		// inbuf + 10 skip header of gz
		ret = lodepng_inflate(&outbuf, &outsize, inbuf + 10, insize, &lodepng_default_decompress_settings);
		if (ret) {
			cerr << "Lodepng error code : " << ret << endl <<
					"Lodepng error message : " << lodepng_error_text(ret) << endl;
		}
		bmphdr = (bmp_hdr*)outbuf;
		bmpinfo = (bmp_hdr_v3*)(outbuf + sizeof(bmp_hdr));
		if (!isValidBmp(bmphdr)) {
			cerr << "# Warning : Extract bmp is invalid." << endl <<
					"# Please Notice if you flash your splash.img but nothing on you screen." << endl;;
		}

		bmpdata.insert(bmpdata.end(), outbuf, outbuf+bmphdr->bfSize);
		if (ConvertToPng) {
			ret = decodeBMP(imagedata, bmpinfo->width, bmpinfo->height, bmpdata);
			// convert bmp to png, skip 0x36 bytes of outbuf, is raw data
			ret = lodepng::encode(pngdata, imagedata, bmpinfo->width, bmpinfo->height);
			if (ret) {
				cerr << "Lodepng error code : " << ret << endl <<
						"Lodepng error message : " << lodepng_error_text(ret) << endl;
			}
		}

		//ret = lodepng_save_file(pngbuf, outsize, OutFile.string().c_str());
		ret = lodepng::save_file(ConvertToPng ? pngdata : bmpdata, OutFile.string());
		if (ret) {
			cerr << "Lodepng error code : " << ret << endl <<
					"Lodepng error message : " << lodepng_error_text(ret) << endl;
		}

		bmpdata.clear();
		if (ConvertToPng) {
			pngdata.clear();
			imagedata.clear();
		}
		free(outbuf);
		free(inbuf);
	}

	close_all();
	cout << "# Done !" << endl <<
			"\tExtract " << dec << splashhdr.imgnumber << " pics into " << fs::absolute(OutDir) << endl;

	return ret;
}

int opsplash::Repack(std::string FilePath, std::string PicPath, std::string OutPath) {
	banner();

	int ret = parseFile(FilePath);
	int fdo, fdi;
	fs::path InputFile  = FilePath, 
			 OutputFile = OutPath,
			 PicDir     = PicPath,
			 PicFile;
	uint8_t *rawbuf, *gzbuf, *image;
	uint32_t width, height;
	uint32_t offset;
	size_t FileSize, gzsize;
	bmp_hdr *bmphdr;
	gzip_header gzhdr = get_def_gziphdr();
	gzip_footer gzftr;
	vector<uint8_t> bmpdata;

	if (!fs::exists(InputFile)) {
		cerr << "Error : File [" << InputFile << "] Does not exist !" << endl;
		return EEXIST;
	}

	// check if you are missing pic
	for (uint32_t i=0; i<splashhdr.imgnumber; i++) {
		PicFile = PicDir;
		PicFile.append((string)(const char*)metadata[i].name + ((ConvertToPng) ? ".png" : ".bmp"));
		if (!fs::exists(PicFile)) {
			cerr << "Error : The [" << PicDir << "] your defined have not pic named [" << PicFile << "]." << endl;
			ret = EEXIST;
			break;
		}
	}
	if (ret)
		return ret;

	fdo = open(OutputFile.string().c_str(), O_CREAT | O_TRUNC | O_BINARY | O_WRONLY);

	if (ddph.magic == DDPH_MAGIC_V1) {
		cout << "# Write DDPH header." << endl;
		lseek(fdo, DDPH_HDR_OFFSET, SEEK_SET);
		write(fdo, &ddph, sizeof(ddph));
	}

	cout << "# Write oppo splash header." << endl;
	lseek(fdo, OPPO_SPLASH_HDR_OFFSET, SEEK_SET);
	write(fdo, &splashhdr, sizeof(splashhdr));


	offset = 0;
	for (uint32_t i=0; i<splashhdr.imgnumber; i++) {
		cout << "# Compress [" << metadata[i].name << "] ..." << endl;
		PicFile = PicDir; PicFile.append((string)(const char*)metadata[i].name + ((ConvertToPng) ? ".png" : ".bmp"));
		FileSize = getFileSize(PicFile.string());

		if (ConvertToPng) {
			ret = lodepng_decode24_file(&image, &width, &height, PicFile.string().c_str());
			if (ret) {
				cerr << "Lodepng error code : " << ret << endl <<
						"Lodepng error message : " << lodepng_error_text(ret) << endl;
			}
			encodeBMP(bmpdata, image, width, height);
		} else {
			bmpdata.resize(FileSize);

			rawbuf = (uint8_t*)malloc(FileSize);
			fdi = open(PicFile.string().c_str(), O_BINARY | O_RDONLY);
			ret = read(fdi, rawbuf, FileSize);
			close(fdi);

			for (uint32_t j = 0; j < FileSize; j++) {
			    bmpdata[j] = rawbuf[j];
			}

			bmphdr = (bmp_hdr*)bmpdata.data();
			if (!isValidBmp(bmphdr)) {
				cerr << "Error : Looks like your bmp file : " << PicFile << endl <<
						"Is not valid bmp file, if format correct, header must be 54 bytes." << endl <<
						"\tyour header size :" << dec << bmphdr->bfoffBits << endl;
				ret = EINVAL;
			}
			free(rawbuf);
		}
		
		gzbuf = (uint8_t*)malloc(sizeof(gzhdr));
		gzsize = sizeof(gzhdr);
		memcpy(gzbuf, &gzhdr, sizeof(gzhdr));
		
		lodepng_deflate(&gzbuf, &gzsize, bmpdata.data(), bmpdata.size(), &lodepng_default_compress_settings);
		gzftr.crc32 = lodepng_crc32(bmpdata.data(), bmpdata.size());
		gzftr.isize = bmpdata.size();
		gzbuf = (uint8_t*)realloc(gzbuf, gzsize + sizeof(gzftr));
		memcpy(gzbuf + gzsize, &gzftr, sizeof(gzftr));
		gzsize += sizeof(gzftr);
		
		metadata[i].offset = offset;
		metadata[i].realsz = bmpdata.size();
		metadata[i].compsz = gzsize;
		lseek(fdo, offset + DATA_OFFSET, SEEK_SET);
		write(fdo, gzbuf, gzsize);

		offset += gzsize;

		if (ConvertToPng)
			free(image);
		free(gzbuf);
		bmpdata.clear();
	}

	// Write metadata
	offset = METADATA_OFFSET;
	for (uint32_t i=0; i<splashhdr.imgnumber; i++) {
		lseek(fdo, offset, SEEK_SET);
		write(fdo, &metadata[i], sizeof(metadata[i]));
		offset += sizeof(metadata[i]);
	}

	
	close(fdo);
	chmod(OutputFile.string().c_str(), 0644);
	close_all();

	cout << "# Done !" << endl <<
			"\tFile saved at : " << fs::absolute(OutputFile) << endl;
	return ret;
}
