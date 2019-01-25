#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>

#include "png.h"

// File Size = 512bytes = COLUMN (4) * PLANE (4) * ROW (16) * PATTERN (2)
#define ROW  0x10
#define PLANE 4
#define PATTERN 2
#define CHARACTOR 3
unsigned long chbuf[CHARACTOR][PATTERN][PLANE][ROW];

#define PNG_HEIGHT (ROW)
#define PNG_WIDTH (CHARACTOR*PATTERN*32)

void _cdecl main(int argc, char **argv);

unsigned __int32 vbuf[ROW][CHARACTOR][PATTERN][4];

void view(void);

void _cdecl main(int argc, char **argv) {
	FILE *fi, *fo;
	static char fpath[_MAX_PATH], opath[_MAX_PATH];
	static char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	while (--argc) {
		if (NULL == (fi = fopen(*++argv, "rb"))) {
			printf("file not exist:%s\n", *argv);
			continue;
		}
		if (1 != fread(chbuf, sizeof(chbuf), 1, fi)) {
			printf("file read error:%s\n", fpath);
			continue;
		}
		fclose(fi);
		_fullpath(fpath, *argv, _MAX_PATH);
		_splitpath(fpath, drive, dir, fname, ext);
		_makepath(opath, drive, dir, fname, ".png");

		if (NULL == (fo = fopen(opath, "wb"))) {
			printf("file open error:%s\n", opath);
			exit(1);
		}
		view();
		unsigned char **image;

		image = (png_bytepp)malloc(PNG_HEIGHT * sizeof(png_bytep));
		for (size_t j = 0; j < PNG_HEIGHT; j++)
			image[j] = (png_bytep)&vbuf[j];
		png_structp png_ptr;
		png_infop info_ptr;
		png_color pal[16] = { {0,0,0}, {0,0,0xFF}, {0xFF,0,0}, {0xFF,0,0xFF}, {0,0xFF,0}, {0,0xFF,0xFF}, {0xFF,0xFF,0}, {0xFF,0xFF,0xFF},
							  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} };

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

void view(void)
{
	int index, i;
	unsigned __int32 aimage;

	for (size_t y = 0; y < ROW; y++)
	for (size_t cy = 0; cy < CHARACTOR; cy++)
	for (size_t pat = 0; pat < PATTERN; pat++)
	{
		memset(&vbuf[y][cy][pat], 0, 16);
		for (i = 0; i < PLANE; i++)
		{
			aimage = chbuf[cy][pat][i][y];
			unsigned __int32 a4[4] = { 0, 0, 0, 0 };
			for (index = 0; index < 32; index++) {
				if (aimage & (1L << index)) {
					a4[index / 8] |= 1L << ((index % 8) * PLANE);
					vbuf[y][cy][pat][0] |= _byteswap_ulong(a4[0]) << i;
					vbuf[y][cy][pat][1] |= _byteswap_ulong(a4[1]) << i;
					vbuf[y][cy][pat][2] |= _byteswap_ulong(a4[2]) << i;
					vbuf[y][cy][pat][3] |= _byteswap_ulong(a4[3]) << i;
				}
			}
		}
	}
}
