#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>

#include "png.h"

#define MAXPAT 0x20
#define ROW  0x8
#define PLANE 4
#define COLUMN 4
#define HEIGHT 3
#define CPAT 8

#define PNG_HEIGHT (HEIGHT*ROW)
#define PNG_WIDTH (CPAT*COLUMN*16)

void _cdecl main(int argc, char **argv);

unsigned short chbuf[MAXPAT][PLANE][ROW];
unsigned __int64 vbuf[HEIGHT][ROW][CPAT][COLUMN];

struct MAP {
	unsigned char mdata[4][CPAT][HEIGHT*2];
	unsigned long d[4];
} map;

#pragma pack(1)
struct rgbq {
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
};
struct header {
	unsigned short bfType;
	unsigned long bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long OffBits;
	unsigned long biSize;
	unsigned long biWidth;
	unsigned long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long biCompression;
	unsigned long biSizeImage;
	unsigned long biXPelsPerMeter;
	unsigned long biYPelsPerMeter;
	unsigned long biClrUsed;
	unsigned long biClrImportant;
	struct rgbq RGB[0x10];
} bHeader = {0x4D42,0xC76,0,0,0x76,
             0x28,0x200,0x30,0x1,0x4,0,0xC00,0,0,16,16,
             {{0,0,0,0},{0xFF,0,0,0},{0,0,0xFF,0},{0xFF,0,0xFF,0},
             {0,0xFF,0,0},{0xFF,0xFF,0,0},{0,0xFF,0xFF,0},{0xFF,0xFF,0xFF,0},
             {0x80,0x80,0x80,0},{0xFF,0,0,0},{0,0,0xFF,0},{0xFF,0,0xFF,0},
             {0,0xFF,0,0},{0xFF,0xFF,0,0},{0,0xFF,0xFF,0},{0xFF,0xFF,0xFF,0}}};
#pragma pack()

void view(void);

void _cdecl main(int argc, char **argv) {
	int x1, x2;
	FILE *fi, *fo;
	static char fpath[_MAX_PATH], opath[_MAX_PATH];
	static char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	while (--argc) {
		if (NULL == (fi = fopen(*++argv,"rb")))	{
			printf("file not exist:%s\n", *argv);
			continue;
		}
		if (sizeof(chbuf) != fread(chbuf,1,sizeof(chbuf),fi)) {
			printf("file read error:%s\n", fpath);
			continue;
		}
		fclose(fi);
		_fullpath(fpath, *argv, _MAX_PATH);
		_splitpath(fpath, drive, dir, fname, ext);
		_makepath(opath, drive, dir, fname, ".png");
		fname[1] = 'P';
		_makepath(fpath, drive, dir, fname, ext);
		if (NULL == (fi = fopen(fpath,"rb"))) {
			printf("file not exist:%s\n", fpath);
			continue;
		}
		if (sizeof(map) != fread(&map,1,sizeof(map),fi)) {
			printf("file read error:%s\n", *argv);
			continue;
		}
		fclose(fi);

		if (NULL == (fo = fopen(opath,"wb"))) {
			printf("file open error:%s\n", opath);
			exit(1);
		}
		view();
		unsigned char **image;

		image = (png_bytepp)malloc(PNG_HEIGHT * sizeof(png_bytep));
		for (size_t j = 0; j < PNG_HEIGHT; j++)
			image[j] = (png_bytep)& vbuf[j / ROW][j % ROW];
		png_structp png_ptr;
		png_infop info_ptr;
		png_color pal[16] = { {0,0,0}, {0,0,0xFF}, {0xFF,0,0}, {0xFF,0,0xFF}, {0,0xFF,0}, {0,0xFF,0xFF}, {0xFF,0xFF,0}, {0xFF,0xFF,0xFF},
							  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}};

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr == NULL) {
			fclose(fo);
			return;
		}
		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL) {
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			fclose(fo);
			return;
		}
		png_init_io(png_ptr, fo);
		png_set_IHDR(png_ptr, info_ptr, PNG_WIDTH, PNG_HEIGHT,
			4, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_set_PLTE(png_ptr, info_ptr, pal, 16);
		png_set_pHYs(png_ptr, info_ptr, 2, 1, PNG_RESOLUTION_UNKNOWN);
		png_byte trans[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
							   0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
		png_set_tRNS(png_ptr, info_ptr, trans, 16, NULL);

		png_write_info(png_ptr, info_ptr);
		png_write_image(png_ptr, image);
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		free(image);
		fclose(fo);
	}
	exit(0);
}

void viewx(unsigned int imagex,int p,int xpos,int ypos);
void view(void)
{
	unsigned int pat,image;
	memset(vbuf,0x88,sizeof(vbuf));

	for (pat=0;pat<CPAT;pat++) {
		image = map.mdata[3][pat][2] & 0x1F;
		viewx(image, pat, 1, 0);
		image = map.mdata[3][pat][3] & 0x1F;
		viewx(image, pat, 2, 0);
		image = map.mdata[3][pat][1] & 0x1F;
		viewx(image, pat, 1, 1);
		image = map.mdata[3][pat][4] & 0x1F;
		viewx(image, pat, 2, 1);
		image = map.mdata[3][pat][0] & 0x1F;
		viewx(image, pat, 1, 2);
		image = map.mdata[3][pat][5] & 0x1F;
		viewx(image, pat, 2, 2);
	}
	viewx(0x1E,4,3,1);
	viewx(0x1E,5,3,1);
	viewx(0x1F,6,0,1);
	viewx(0x1F,7,0,1);
}
void viewx(unsigned int imagex,int p,int xpos,int ypos)
{
	int index, i, y;
	unsigned short aimage;

	for (y=0;y<ROW;y++) {
		vbuf[ypos][y][p][xpos] = 0LL;
		for (i=0;i<PLANE;i++)
		{
			aimage = chbuf[imagex][i][y];
			union _t {
				unsigned __int64 a8;
				unsigned __int32 a4[2];
			} u;
			u.a8 = 0;
			for (index = 0; index < 16; index++) {
				if (aimage & (1 << index)) {
					u.a8 |= 1LL << (index * PLANE);
					vbuf[ypos][y][p][xpos] |= ((unsigned __int64)_byteswap_ulong(u.a4[0]) | ((unsigned __int64)_byteswap_ulong(u.a4[1]) << 32)) << i;
				}
			}
		}
	}
}
