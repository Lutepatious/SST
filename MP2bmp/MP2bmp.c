/* sorcerian map view program */
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <signal.h>
#include <memory.h>

#include "png.h"

#define DLENGTH 0x0C
#define BMAX	0x80
#define CMAX	0x80
#define CTMAX	0x50
#define PLANE	0x4
#define ROW	0x8
#define DISPW	0x14
#define FULLBUFCOL (unsigned int)(BMAX*2)
#define FULLBUFROW (unsigned int)(DLENGTH*ROW*2)

#define PNG_HEIGHT (CTMAX*2)
#define PNG_WIDTH (FULLBUFCOL)

unsigned short cbuf[CMAX][PLANE][ROW];	/* 8192 byte */
unsigned char mbuf[BMAX][DLENGTH];	/* 1536 byte */
unsigned char ctbuf[CTMAX][2][2];	/*  320 byte */

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
	struct rgbq RGB0;
	struct rgbq RGB1;
	struct rgbq RGB2;
	struct rgbq RGB3;
	struct rgbq RGB4;
	struct rgbq RGB5;
	struct rgbq RGB6;
	struct rgbq RGB7;
} bHeader = {0x4D42,0xA056,0,0,0x56,
             0x28,0x100,0x140,0x1,0x4,0,0xA000,0,0,8,8,
             {0,0,0,0},{0xFF,0,0,0},{0,0,0xFF,0},{0xFF,0,0xFF,0},
             {0,0xFF,0,0},{0xFF,0xFF,0,0},{0,0xFF,0xFF,0},{0xFF,0xFF,0xFF,0}};
#pragma pack()

unsigned __int64 bm[160][16];

void main(int argc, char **argv);
void __cdecl ctrlc(int a);
void viewmap(void);


void __cdecl ctrlc(int a) {
	puts("CTRL+C Pressed!!\n");
	exit(1);
}

void main(int argc,char **argv)
{
	FILE *fp;
	static char fpath[_MAX_PATH], cfile[_MAX_PATH], ofile[_MAX_PATH];
	static char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	int y;

	signal(SIGINT, ctrlc);
	if (argc < 2) {
		printf("Usage:%s map-file\n", *argv);
		exit(-1);
	}

	while (--argc) {
		if (NULL==(fp=fopen(*++argv,"rb"))) {
			printf("file open error!! %s\n", *argv);
			continue;
		}
		if (fread(&mbuf,1,sizeof(mbuf),fp)!=sizeof(mbuf) || fread(&ctbuf,1,sizeof(ctbuf),fp)!=sizeof(ctbuf)) {
			puts("map-file read error!!\n");
			continue;
		}
		fclose(fp);
		_fullpath(fpath, *argv, _MAX_PATH);
		_splitpath(fpath, drive, dir, fname, ext);
		fname[1] = 'P';
		_makepath(ofile, drive, dir, fname, ".png");
		fname[0] = 'C';
		fname[1] = '_';
		fname[4] = '0';
		_makepath(cfile, drive, dir, fname, ext);
		if (NULL==(fp=fopen(cfile,"rb"))) {
			printf("file open error!! %s\n", cfile);
			continue;
		}
		if (fread(&cbuf,1,0x2000,fp)!=0x2000) {
			puts("colour-file read error!!\n");
			continue;
		}
		fclose(fp);
		if (NULL==(fp=fopen(ofile,"wb"))) {
			printf("file open error!! %s\n", ofile);
			exit(-1);
		}
		viewmap();
		unsigned char **image;

		image = (png_bytepp)malloc(PNG_HEIGHT * sizeof(png_bytep));
		for (size_t j = 0; j < PNG_HEIGHT; j++)
			image[j] = (png_bytep)&bm[j];

		png_structp png_ptr;
		png_infop info_ptr;
		png_color pal[8] = { {0,0,0}, {0,0,0xFF}, {0xFF,0,0}, {0xFF,0,0xFF}, {0,0xFF,0}, {0,0xFF,0xFF}, {0xFF,0xFF,0}, {0xFF,0xFF,0xFF} };

		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr == NULL) {
			fclose(fp);
			return;
		}
		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL) {
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			fclose(fp);
			return;
		}
		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, info_ptr, PNG_WIDTH, PNG_HEIGHT,
			4, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_set_PLTE(png_ptr, info_ptr, pal, 8);
		png_set_pHYs(png_ptr, info_ptr, 2, 1, PNG_RESOLUTION_UNKNOWN);
		png_write_info(png_ptr, info_ptr);
		png_write_image(png_ptr, image);
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		free(image);
		fclose(fp);
	}
	exit(0);
}

#define X1(x)	(unsigned int) ((x)>>1)
#define X2(x)	(unsigned int) ((x)&1)

#define Y1(y)	(unsigned int) (((y)/ROW)>>1)
#define Y2(y)	(unsigned int) (((y)/ROW)&1)
#define Y3(y)	(unsigned int) ((y)%ROW)
#define WIDTH 0x10

void viewmap(void)
{
	unsigned short aimage;
	unsigned int x, x1, x2, y, image, i, index;

	memset(bm,0,sizeof(bm));

	for (x=0;x< WIDTH;x++) { /* rows 256 pixel */
		x1=X1(x);
		x2=X2(x);
		for(y=0;y< CTMAX/4*ROW ;y++) { /* columns 160 pixel */
			image=ctbuf[x1+Y1(y)*0x8][x2][Y2(y)]&0x7F;
			for(i=0;i<PLANE-1;i++) {
				aimage=cbuf[image][i][Y3(y)];
				union _t {
					unsigned __int64 a8;
					unsigned __int32 a4[2];
				} u;
				u.a8 = 0;

				for (index=0;index<16;index++) {
					if (aimage & (1 << index)) {
						u.a8 |= 1LL << (index * PLANE);
						bm[y][x] |= ((unsigned __int64)_byteswap_ulong(u.a4[0]) | ((unsigned __int64)_byteswap_ulong(u.a4[1]) << 32)) << i;
					}
				}
			}
		}
	}
}
