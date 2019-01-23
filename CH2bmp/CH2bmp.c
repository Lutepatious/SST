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
 {"CH000.BMP","CH001.BMP","CH002.BMP"},{"CH010.BMP","CH011.BMP","CH012.BMP"},
 {"CH100.BMP","CH101.BMP","CH102.BMP"},{"CH110.BMP","CH111.BMP","CH112.BMP"},
 {"CH200.BMP","CH201.BMP","CH202.BMP"},{"CH210.BMP","CH211.BMP","CH212.BMP"},
 {"CH300.BMP","CH301.BMP","CH302.BMP"},{"CH310.BMP","CH311.BMP","CH312.BMP"}};

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
