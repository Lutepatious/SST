#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "png.h"

#define ROW 8
#define PLANE 4
#define BANK 16
#define COLUMN 16

#define PNG_HEIGHT (ROW*BANK)
#define PNG_WIDTH (COLUMN*16)


unsigned short cbuf[BANK][COLUMN][PLANE-1][ROW];
unsigned __int64 buf[BANK][ROW][COLUMN];

void __cdecl main(int argc, char **argv);
void viewmap(void);

void __cdecl main(int argc, char **argv)
{
	FILE *fi, *fo;
	int i;
	size_t rsize;

	static char fpath[_MAX_PATH];
	static char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	while(--argc) {
		if (NULL == (fi = fopen(*++argv,"rb"))) {
			printf("file open error!! %s\n", *argv);
			continue;
		}
		_fullpath(fpath, *argv, _MAX_PATH);
		_splitpath(fpath, drive, dir, fname, ext);
		_makepath(fpath, drive, dir, fname, ".png");
		if (0 == (rsize = fread(cbuf,sizeof(cbuf),1,fi))) {
			printf("file read error!! %s\n", *argv);
			continue;
		}
		fclose(fi);

		if (NULL == (fo = fopen(fpath,"wb"))) {
			printf("file open error!! %s\n", fpath);
			exit(-1);
		}
		viewmap();
		unsigned char **image;

		image = (png_bytepp)malloc(PNG_HEIGHT * sizeof(png_bytep));
		for (size_t j = 0; j < PNG_HEIGHT; j++)
			image[j] = (png_bytep)&buf[j/ROW][j%ROW];

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
}

void viewmap(void)
{
	int index;
	unsigned short aimage;

	memset(buf,0,sizeof(buf));

	for (size_t j=0;j<BANK;j++)
	for (size_t y=0;y<ROW;y++)
	for (size_t x = 0; x < COLUMN; x++)
	for (size_t i=0;i<PLANE-1;i++) {
		aimage = cbuf[j][x][i][y];
		union _t {
			unsigned __int64 a8;
			unsigned __int32 a4[2];
		} u;
		u.a8 = 0;
		/* byte swap */
		/* PC-98 plane bitmap
		 * 0000000011111111
		 * 7654321076543210
		 * BMP file 4bit packedpixel bitmap
		 * 11110000333322225555444477776666
		 * abrgabrgabrgabrgabrgabrgabrgabrg
		 */
		for (index=0;index<16;index++) {
			if (aimage & (1 << index)) {
				u.a8 |= 1LL << (index * PLANE);
				buf[j][y][x] |= ((unsigned __int64)_byteswap_ulong(u.a4[0]) | ((unsigned __int64)_byteswap_ulong(u.a4[1]) << 32)) << i;
			}
		}
	}
}
