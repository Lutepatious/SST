/* sorcerian character view program */
#include <stdio.h>
#include <stdlib.h>

#define PBSIZE	0x8
#define CMAX	0x100
#define PLANE	0x4
#define ROW	0x8

#define COLUMN	3
#define HEIGHT	3
#define CPAT	4
#define CPAT2	0x10
#define PMAX	CPAT*CPAT2

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
} bHeader = {0x4D42,0x12076,0,0,0x76,
             0x28,0xC0,0x300,0x1,0x4,0,0x12000,0,0,16,16,
             {{0,0,0,0},{0xFF,0,0,0},{0,0,0xFF,0},{0xFF,0,0xFF,0},
             {0,0xFF,0,0},{0xFF,0xFF,0,0},{0,0xFF,0xFF,0},{0xFF,0xFF,0xFF,0},
             {0x80,0x80,0x80,0},{0xFF,0,0,0},{0,0,0xFF,0},{0xFF,0,0xFF,0},
             {0,0xFF,0,0},{0xFF,0xFF,0,0},{0,0xFF,0xFF,0},{0xFF,0xFF,0xFF,0}}};
#pragma pack()

unsigned short cbuf[CMAX][PLANE][ROW];	/* 16384 byte */
unsigned char pbuf[PMAX][PBSIZE];	/* 512 byte */
unsigned long vbuf[CPAT2][HEIGHT][ROW][CPAT][COLUMN][2];

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
			fprintf(stdout, "file open error!! %s", argv);
		if (fread(&pbuf,1,0x200,fp)!=0x200) {
			fputs("pattern file read error!!\n", stderr);
			exit(2);
		}
		fclose(fp);
		_fullpath(fpath, *argv, _MAX_PATH);
		_splitpath(fpath, drive, dir, fname, ext);
		_makepath(ofile, drive, dir, fname, ".BMP");
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
		if (NULL==(fp=fopen(ofile,"wb")))
			fprintf(stdout, "file open error!! %s", ofile);
		imout();
		fwrite(&bHeader,1,sizeof(bHeader),fp);
		for (x0=CPAT2-1;x0>=0;x0--)
		for (x1=HEIGHT-1;x1>=0;x1--)
		for (x2=ROW-1;x2>=0;x2--) {
			fwrite(&vbuf[x0][x1][x2],1, 0x60,fp);
			fwrite(&vbuf[x0][x1][x2],1, 0x60,fp);
		}
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
	int index, i, y, px, py, qx, qy;
	unsigned short aimage;

	px=p%CPAT;
	py=p/CPAT;

	for (y=0;y<ROW;y++) {
		vbuf[py][ypos][y][px][xpos][1] = vbuf[py][ypos][y][px][xpos][0] = 0L;
		for (i=0;i<PLANE;i++)
		{
			aimage = cbuf[imagex][i][y];
			for (index=0;index<16;index++) {
				if (aimage & (1 << (15-(index^0x8))))
					vbuf[py][ypos][y][px][xpos][index>>3] |= 1L << (((index&7^1)<<2)+i);
			}
		}
	}
}
