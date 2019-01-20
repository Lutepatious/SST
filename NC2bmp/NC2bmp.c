#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#define MAXPAT 0x20
#define ROW  0x8
#define PLANE 4
#define COLUMN 4
#define HEIGHT 3
#define CPAT 8

void _cdecl main(int argc, char **argv);

unsigned short chbuf[MAXPAT][PLANE][ROW];
unsigned long vbuf[HEIGHT][ROW][CPAT][COLUMN][2];

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
		_makepath(opath, drive, dir, fname, ".BMP");
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

		fwrite(&bHeader,1,sizeof(bHeader),fo);
		for (x1=HEIGHT-1;x1>=0;x1--)
		for (x2=ROW-1;x2>=0;x2--) {
			fwrite(&vbuf[x1][x2],1, 0x100,fo);
			fwrite(&vbuf[x1][x2],1, 0x100,fo);
		}
		fclose(fo);
	}
	exit(0);
}

/* unsigned long vbuf[HEIGHT][ROW][CPAT][COLUMN][2]; */
/* unsigned short chbuf[MAXPAT][PLANE][ROW]; */
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
		vbuf[ypos][y][p][xpos][1] = vbuf[ypos][y][p][xpos][0] = 0L;
		for (i=0;i<PLANE;i++)
		{
			aimage = chbuf[imagex][i][y];
			for (index=0;index<16;index++) {
				if (aimage & (1 << (15-(index^0x8))))
					vbuf[ypos][y][p][xpos][index>>3] |= 1L << (((index&7^1)<<2)+i);
			}
		}
	}
}
