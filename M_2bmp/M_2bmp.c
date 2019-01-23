/* sorcerian map view program */
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
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

#define PNG_HEIGHT (FULLBUFROW)
#define PNG_WIDTH (FULLBUFCOL*16)

unsigned short cbuf[CMAX][PLANE][ROW];	/* 8192 byte */
unsigned char mbuf[BMAX][DLENGTH];	/* 1536 byte */
unsigned char ctbuf[CTMAX][2][2];	/*  320 byte */

unsigned __int64 bm[FULLBUFROW][FULLBUFCOL];

void main(int argc, char **argv);
void __cdecl ctrlc(int a);
void viewmap(int a);


void __cdecl ctrlc(int a) {
	puts("CTRL+C Pressed!!\n");
	exit(1);
}

void main(int argc,char **argv)
{
	FILE *fp;
	static char fpath[_MAX_PATH], cfile[_MAX_PATH], ofile[_MAX_PATH];
	static char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	static char fname1[_MAX_FNAME]={0};
	int y, a;

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
		strcpy(fname1,fname);
		fname[0] = 'C';
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
		for (a=0;a<5;a++) {
			fname1[5]='0'+a;
			_makepath(ofile, drive, dir, fname1, ".png");
			if (NULL==(fp=fopen(ofile,"wb"))) {
				printf("file open error!! %s\n", ofile);
				exit(-1);
			}
			viewmap(a);
			unsigned char **image;

			image = (png_bytepp)malloc(PNG_HEIGHT * sizeof(png_bytep));
			for (size_t j = 0; j < PNG_HEIGHT; j++)
				image[j] = (png_bytep)&bm[j];

			png_structp png_ptr;
			png_infop info_ptr;
			png_color pal[8] = { {0,0,0}, {0,0,0xFF}, {0xFF,0,0}, {0xFF,0,0xFF}, {0,0xFF,0}, {0,0xFF,0xFF}, {0xFF,0xFF,0}, {0xFF,0xFF,0xFF}};

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
	}
	exit(0);
}

#define X1(x)	(unsigned int) ((x)>>1)
#define X2(x)	(unsigned int) ((x)&1)

#define Y1(y)	(unsigned int) (((y)/ROW)>>1)
#define Y2(y)	(unsigned int) (((y)/ROW)&1)
#define Y3(y)	(unsigned int) ((y)%ROW)


void viewmap(int a)
{
	unsigned short aimage;
	unsigned int x, x1, x2, y, image, i, index, pattern;

	memset(bm,0,sizeof(bm));

	for (x=0;x< FULLBUFCOL;x++) { /* rows 4096 pixel */
		x1=X1(x);
		x2=X2(x);
		for(y=0;y< FULLBUFROW;y++) { /* columns 192 pixel */
			/* pattern <- mbuf[X1(x)][Y1(y)] */
			pattern = mbuf[x1][Y1(y)];
			if (pattern >= 0x3c)
				pattern += 4*a;
			/* image <- ctbuf[pattern][X2(x)][Y2(y)] */
			image=ctbuf[pattern][x2][Y2(y)]&0x7F;
			for(i=0;i<PLANE-1;i++) {
				/* aimage <- cbuf[image][plane][Y3(y)] */
				aimage=cbuf[image][i][Y3(y)];
				union _t {
					unsigned __int64 a8;
					unsigned __int32 a4[2];
				} u;
				u.a8 = 0;

				/* byte swap */
				/* PC-98 plane bitmap
				 * 76543210fedcba98
				 * BMP file 4bit packedpixel bitmap
				 * 1111000033332222555544447777666699998888bbbbaaaaddddccccffffeeee
				 * abrgabrgabrgabrgabrgabrgabrgabrgabrgabrgabrgabrgabrgabrgabrgabrg
				 */
				for (index=0;index<16;index++) {
					if (aimage & (1 << index)) {
						u.a8 |= 1LL << (index * PLANE);
						bm[y][x] |= ((unsigned __int64)_byteswap_ulong(u.a4[0]) | ((unsigned __int64) _byteswap_ulong(u.a4[1]) << 32)) << i;
					}
				}
			}
		}
	}
}
