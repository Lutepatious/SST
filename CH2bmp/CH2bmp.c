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

struct AGE {
	unsigned char mdata[CPAT][HEIGHT*2];
	unsigned long d[4];
};

struct MAP {
	struct AGE age[4];
} map[8];

unsigned char *CHFILE[8][3] = {
 {"CH000","CH001","CH002"},{"CH010","CH011","CH012"},
 {"CH100","CH101","CH102"},{"CH110","CH111","CH112"},
 {"CH200","CH201","CH202"},{"CH210","CH211","CH212"},
 {"CH300","CH301","CH302"},{"CH310","CH311","CH312"}};

unsigned char *BMPFILE[8][3] = {
 {"CH000.png","CH001.png","CH002.png"},{"CH010.png","CH011.png","CH012.png"},
 {"CH100.png","CH101.png","CH102.png"},{"CH110.png","CH111.png","CH112.png"},
 {"CH200.png","CH201.png","CH202.png"},{"CH210.png","CH211.png","CH212.png"},
 {"CH300.png","CH301.png","CH302.png"},{"CH310.png","CH311.png","CH312.png"}};

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

unsigned char *MAPFILE="PLPTR";
void view(int cnum,int cage);

void _cdecl main(int argc, char **argv) {
	int i, j;
	int x1, x2;
	FILE *fi, *fo;

	if (NULL == (fi = fopen(MAPFILE,"rb")))	{
		printf("file not exist:%s\n", MAPFILE);
		exit(1);
	}
	if (sizeof(map) != fread(map,1,sizeof(map),fi)) {
		printf("file read error:%s\n", MAPFILE);
		exit(2);
	}
	fclose(fi);

	for (j=0;j<8;j++)
	for (i=0;i<3;i++) {
		if (NULL == (fi = fopen(CHFILE[j][i],"rb"))) {
			printf("file not exist:%s\n", CHFILE[j][i]);
			exit(1);
		}
		if (sizeof(chbuf) != fread(chbuf,1,sizeof(chbuf),fi)) {
			printf("file read error:%s\n", CHFILE[j][i]);
			exit(2);
		}
		fclose(fi);
		if (NULL == (fo = fopen(BMPFILE[j][i],"wb"))) {
			printf("file open error:%s\n", BMPFILE[j][i]);
			exit(1);
		}
		view(j,i);
		unsigned char **image;

		image = (png_bytepp)malloc(PNG_HEIGHT * sizeof(png_bytep));
		for (size_t j = 0; j < PNG_HEIGHT; j++)
			image[j] = (png_bytep)& vbuf[j / ROW][j % ROW];
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

void viewx(unsigned int imagex,int p,int xpos,int ypos);
void view(int cnum,int cage)
{
	unsigned int pat,image;
	memset(vbuf,0x88,sizeof(vbuf));

	for (pat=0;pat<CPAT;pat++) {
		image = map[cnum].age[cage].mdata[pat][2];
		viewx(image, pat, 1, 0);
		image = map[cnum].age[cage].mdata[pat][3];
		viewx(image, pat, 2, 0);
		image = map[cnum].age[cage].mdata[pat][1];
		viewx(image, pat, 1, 1);
		image = map[cnum].age[cage].mdata[pat][4];
		viewx(image, pat, 2, 1);
		image = map[cnum].age[cage].mdata[pat][0];
		viewx(image, pat, 1, 2);
		image = map[cnum].age[cage].mdata[pat][5];
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
		for (i=0;i<PLANE;i++) {
			aimage = chbuf[imagex][i][y];
			union _t {
				unsigned __int64 a8;
				unsigned __int32 a4[2];
			} u;
			u.a8 = 0;
			for (index=0;index<16;index++) {
				if (aimage & (1 << index)) {
					u.a8 |= 1LL << (index * PLANE);
					vbuf[ypos][y][p][xpos] |= ((unsigned __int64)_byteswap_ulong(u.a4[0]) | ((unsigned __int64)_byteswap_ulong(u.a4[1]) << 32)) << i;
				}
			}
		}
	}
}
