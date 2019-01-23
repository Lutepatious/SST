/* sorcerian character view program */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <memory.h>
#include <sys/types.h>

#include "png.h"

#define PBSIZE	0x8
#define CMAX	0x100
#define PLANE	0x4
#define ROW	0x8

#define COLUMN	3
#define HEIGHT	3
#define CPAT	4
#define CPAT2	0x10
#define PMAX	CPAT*CPAT2

#define PNG_HEIGHT (CPAT2*HEIGHT*ROW)
#define PNG_WIDTH (CPAT*COLUMN*16)

unsigned short cbuf[CMAX][PLANE][ROW];	/* 16384 byte */
unsigned char pbuf[PMAX][PBSIZE];	/* 512 byte */
unsigned __int64 vbuf[CPAT2][HEIGHT][ROW][CPAT][COLUMN];

int _cdecl  main(int argc, char **argv);
void imout(void);

short cfset=0;

int _cdecl main(int argc,char **argv)
{
	FILE *fp;
	static char fpath[_MAX_PATH], ofile[_MAX_PATH];
	static char cfile[_MAX_PATH], cfile1[_MAX_PATH], cfile2[_MAX_PATH];
	static char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	int x0,x1,x2;

	memset(&cbuf,0,sizeof(cbuf));
	for (x0=0;x0<CMAX;x0++)
		memset(&cbuf[x0][3],0xFF,sizeof(unsigned short)*ROW);
	while(--argc) {
		if (NULL==(fp=fopen(*++argv,"rb")))
			fprintf(stdout, "file open error!! %s", *argv);
		if (fread(&pbuf,1,0x200,fp)!=0x200) {
			fputs("pattern file read error!!\n", stderr);
			exit(2);
		}
		fclose(fp);
		_fullpath(fpath, *argv, _MAX_PATH);
		_splitpath(fpath, drive, dir, fname, ext);
		_makepath(ofile, drive, dir, fname, ".png");
		fname[0] = 'C';
		_makepath(cfile1, drive, dir, fname, ext);
		fname[4]++;
		_makepath(cfile2, drive, dir, fname, ext);
#if 0
		fname[4] = '0';
		_makepath(cfile, drive, dir, fname, ext);	
		if (NULL==(fp=fopen(cfile,"rb")))
			fprintf(stdout, "file open error!! %s", cfile);
		if (fread(&cbuf[0x00],1,0x2000,fp)!=0x2000) {
			fputs("colour-file read error!!\n", stderr);
			exit(2);
		}
		fclose(fp);
#endif
		if (NULL==(fp=fopen(cfile1,"rb")))
			fprintf(stdout, "file open error!! %s", cfile1);
		if (fread(&cbuf[0xA0],1,0x1800,fp)!=0x1800) {
			fputs("colour-file1 read error!!\n", stderr);
			exit(2);
		}
		fclose(fp);
		if (NULL!=(fp=fopen(cfile2,"rb"))) {
			if (fread(&cbuf[0x60],1,0x1000,fp)!=0x1000) {
				fputs("colour-file2 read error!!\n", stderr);
				exit(2);
			}
			fclose(fp);
		}

		if (NULL == (fp = fopen(ofile, "wb")))
			fprintf(stdout, "file open error!! %s", ofile);

		imout();
		unsigned char **image;

		image = (png_bytepp)malloc(PNG_HEIGHT * sizeof(png_bytep));
		for (size_t j = 0; j < PNG_HEIGHT; j++)
			image[j] = (png_bytep)& vbuf[j / (ROW*HEIGHT)][(j / ROW) % HEIGHT][j % ROW];

		png_structp png_ptr;
		png_infop info_ptr;
		png_color pal[16] = { {0,0,0}, {0,0,0xFF}, {0xFF,0,0}, {0xFF,0,0xFF}, {0,0xFF,0}, {0,0xFF,0xFF}, {0xFF,0xFF,0}, {0xFF,0xFF,0xFF},
							  {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} };

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
		fclose(fp);
	}
	exit(0);
}

void viewx(unsigned int imagex,int p,int xpos,int ypos);
void view(int n,int f,int p,int xpos,int ypos)
{
	unsigned int i=pbuf[n+f][p];
	if (0x00==i || 0xFF==i)
		return;
	viewx(i,n,xpos,ypos);
}

void imout(void)
{
	unsigned int a,j=0;
	memset(&vbuf,0x88,sizeof(vbuf));

	for (a=0;a<PMAX;a++) {
		if (j!=0) goto c6;
		switch(pbuf[a][0]) {
			case 0:
				view(a,0,1,0,2);
				break;
			case 1:
				view(a,0,2,0,1);
				view(a,0,1,0,2);
				view(a,0,3,1,1);
				view(a,0,4,1,2);
				break;
			case 2:
				view(a,0,3,0,0);
				view(a,0,2,0,1);
				view(a,0,1,0,2);
				view(a,0,4,1,0);
				view(a,0,5,1,1);
				view(a,0,6,1,2);
				break;
			case 3:
				view(a,0,6,0,1);
				view(a,0,1,0,2);
				view(a,0,5,1,1);
				view(a,0,2,1,2);
				view(a,0,4,2,1);
				view(a,0,3,2,2);
				break;	
			case 4:		
			/*	view(a,0,3,0,0); */
				view(a,0,2,0,1);
				view(a,0,1,0,2);
				break;	
			case 5:		
				view(a,0,1,0,2);
				view(a,0,2,1,2);
				view(a,0,3,2,2);
				break;
			case 6:
				j=1;
c6:
				view(a,0,1,0,5);
				view(a,0,2,0,4);
				view(a,0,3,0,3);
				view(a,0,4,0,2);
				view(a,0,5,0,1);
				view(a,0,6,0,0);
				view(a,0,7,1,0);
				view(a,1,0,1,1);
				view(a,1,1,1,2);
				view(a,1,2,1,3);
				view(a,1,3,1,4);
				view(a,1,4,1,5);
				view(a,1,5,2,5);
				view(a,1,6,2,4);
				view(a,1,7,2,3);
				view(a,2,0,2,2);
				view(a,2,1,2,1);
				view(a,2,2,2,0);
				view(a,2,3,3,0);
				view(a,2,4,3,1);
				view(a,2,5,3,2);
				view(a,2,6,3,3);
				view(a,2,7,3,4);
				view(a,3,0,3,5);
				view(a,3,1,4,5);
				view(a,3,2,4,4);
				view(a,3,3,4,3);
				view(a,3,4,4,2);
				view(a,3,5,4,1);
				view(a,3,6,4,0);
				view(a,3,7,5,0);
				view(a,4,0,5,1);
				view(a,4,1,5,2);
				view(a,4,2,5,3);
				view(a,4,3,5,4);
				view(a,4,4,5,5);
				a+=7;
				break;
			default:
				break;
		}
	}
}

void viewx(unsigned int imagex,int p,int xpos,int ypos)
{
	int index, i, y, px, py;
	unsigned short aimage;

	px=p%CPAT;
	py=p/CPAT;

	for (y=0;y<ROW;y++) {
		vbuf[py][ypos][y][px][xpos] = 0L;
		for (i=0;i<PLANE;i++)
		{
			aimage = cbuf[imagex][i][y];
			union _t {
				unsigned __int64 a8;
				unsigned __int32 a4[2];
			} u;
			u.a8 = 0;
			for (index=0;index<16;index++) {
				if (aimage & (1 << index)) {
					u.a8 |= 1LL << (index * PLANE);
					vbuf[py][ypos][y][px][xpos] |= ((unsigned __int64)_byteswap_ulong(u.a4[0]) | ((unsigned __int64)_byteswap_ulong(u.a4[1]) << 32)) << i;
				}
			}
		}
	}
}
