#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define ROW 8
#define COLUMN 0x10
#define PLANE 4
#define BANK 8
#define BSIZE (ROW*COLUMN*sizeof(unsigned long)*2)

unsigned short cbuf[COLUMN*BANK][PLANE][ROW];
unsigned long buf[ROW*BANK][COLUMN][2];

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
} bHeader = {0x4D42,0x76,0,0,0x76,
             0x28,0x100,0,0x1,0x4,0,0,0,0,16,16,
             {{0,0,0,0},{0xFF,0,0,0},{0,0,0xFF,0},{0xFF,0,0xFF,0},
             {0,0xFF,0,0},{0xFF,0xFF,0,0},{0,0xFF,0xFF,0},{0xFF,0xFF,0xFF,0},
             {0x80,0x80,0x80,0},{0xFF,0,0,0},{0,0,0xFF,0},{0xFF,0,0xFF,0},
             {0,0xFF,0,0},{0xFF,0xFF,0,0},{0,0xFF,0xFF,0},{0xFF,0xFF,0xFF,0}}};
#pragma pack()

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
		_makepath(fpath, drive, dir, fname, ".BMP");
		if (0 == (rsize = fread(cbuf,BSIZE,BANK,fi))) {
			printf("file read error!! %s\n", *argv);
			continue;
		}
		fclose(fi);

		if (NULL == (fo = fopen(fpath,"wb"))) {
			printf("file open error!! %s\n", fpath);
			exit(-1);
		}
		fwrite(&bHeader,1,sizeof(bHeader),fo);
		viewmap();
		for (i=rsize*ROW-1;i>=0;i--) {
			fwrite(&buf[i],1,COLUMN*8,fo);
			fwrite(&buf[i],1,COLUMN*8,fo);
		}
		bHeader.bfSize = rsize*2*BSIZE + 0x76;
		bHeader.biSizeImage = rsize*2*BSIZE;
		bHeader.biHeight = rsize*2*ROW;
		fseek(fo,0,SEEK_SET);
		fwrite(&bHeader,1,sizeof(bHeader),fo);
		fclose(fo);
	}
}

void viewmap(void)
{
	int x,y,i,j,index;
	unsigned short aimage;

	memset(buf,0,sizeof(buf));

	for (j=0;j<BANK;j++)
	for (y=0;y<ROW;y++)
	for (x=0;x<COLUMN;x++)
	for (i=0;i<PLANE;i++) {
		aimage = cbuf[x+j*COLUMN][i][y];
		for (index=0;index<16;index++) {
			/* byte swap */
			/* PC-98 plane bitmap
			 * 0000000011111111
			 * 7654321076543210
			 * BMP file 4bit packedpixel bitmap
			 * 11110000333322225555444477776666
			 * abrgabrgabrgabrgabrgabrgabrgabrg
			 */
			if (aimage & (1 << (15-(index^0x8))))
				buf[y+j*ROW][x][index>>3] |= 1L << (((index&7^1)<<2)+i);
		}
	}
}
