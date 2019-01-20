/* sorcerian character view program */
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <share.h>
#include <signal.h>
#include <graph.h>


#define PBSIZE	0x8
#define PMAX	0x40
#define CMAX	0x100
#define PLANE	0x4
#define ROW	0x8
#define DISPW	0x14
#define VIWW	0x10
#define BVUS	unsigned short _based(void)
#define FULLBUFCOL (unsigned short)(BMAX*2)
#define FULLBUFROW (unsigned short)(DLENGTH*ROW*2)

unsigned short cbuf[CMAX][PLANE][ROW]={0};	/* 16384 byte */
unsigned char pbuf[PMAX][PBSIZE];	/* 512 byte */

_segment p[PLANE] = {0xA800,0xB800,0xB000,0xE000};

void gclear(void);
void main(short argc, char **argv);
void ctrlc(void);
void imout(void);
void _fastcall _imout(unsigned short y,BVUS *bp,unsigned short x);

void ctrlc(void) {
	gclear();
	fputs("CTRL+C Pressed!!\n", stderr);
	exit(1);
}

void gclear(void)
{
	_setactivepage(1);
	_clearscreen(_GCLEARGRAPH);
	_setactivepage(0);
	_clearscreen(_GCLEARGRAPH);
	_setvisualpage(0);
	_displaycursor(_GCURSORON);
	_setvideomode(_98TEXT80);
}

void main(short argc,char **argv)
{
	FILE *fp;
	char fpath[_MAX_PATH];
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];	char save;

	signal(SIGINT, ctrlc);
	if (argc < 2) {
		fprintf(stderr, "Usage:%s [pattern-file.....]\n", *argv);
		exit(-1);
	}
	_setvideomode(_98RESS16COLOR);
	_displaycursor(_GCURSOROFF);

	while(--argc) {
		if (NULL==(fp=_fsopen(*++argv,"rb",SH_DENYWR)))
			fprintf(stdout, "\x1b[25;1Hfile open error!! %s", *argv);
		if (fread(&pbuf,1,0x200,fp)!=0x200) {
			fputs("pattern file read error!!\n", stderr);
			_displaycursor(_GCURSORON);
			_setvideomode(_98TEXT80);
			exit(2);
		}
		fclose(fp);
		fprintf(stdout, "\x1b[25;40H%s", *argv);
		_fullpath(fpath, *argv, _MAX_PATH);
		_splitpath(fpath, drive, dir, fname, ext);
		fname[0] = 'C';
		_makepath(fpath, drive, dir, fname, ext);
		if (NULL==(fp=_fsopen(fpath,"rb",SH_DENYWR)))
			fprintf(stdout, "\x1b[25;1Hfile open error!! %s", *argv);
		if (fread(&cbuf[0xA0],1,0x1800,fp)!=0x1800) {
			fputs("colour-file2 read error!!\n", stderr);
			_displaycursor(_GCURSORON);
			_setvideomode(_98TEXT80);
			exit(2);
		}
		fclose(fp);
		save = fname[4];
		fname[4] = '0';
		_makepath(fpath, drive, dir, fname, ext);
		if (NULL==(fp=_fsopen(fpath,"rb",SH_DENYWR)))
			fprintf(stdout, "\x1b[25;1Hfile open error!! %s", *argv);
		if (fread(&cbuf,1,0x2000,fp)!=0x2000) {
			fputs("colour-file read error!!\n", stderr);
			_displaycursor(_GCURSORON);
			_setvideomode(_98TEXT80);
			exit(2);
		}
		fclose(fp);
		fname[4] = ++save;
		_makepath(fpath, drive, dir, fname, ext);
		if (NULL!=(fp=_fsopen(fpath,"rb",SH_DENYWR))) {
			if (fread(&cbuf[0x60],1,0x1000,fp)!=0x1000) {
				fputs("colour-file2 read error!!\n", stderr);
				_displaycursor(_GCURSORON);
				_setvideomode(_98TEXT80);
				exit(2);
			}
			fclose(fp);
		}
		imout();
		while(!kbhit());
		getch();
		_clearscreen(_GCLEARSCREEN);
	}
	gclear();
	exit(0);
}

#define BP(x,y) (bp+(x)*DISPW*2*2*ROW+(y))

void imout(void)
{
	BVUS *bp;
	unsigned short i,j=0;

	for (i=0;i<PMAX;i++) {
		bp=(BVUS *)((DISPW*2*(ROW*2*3)*((i/4)%8)+3*(i%4+4*(i/4/8)))*2);
		if (j!=0) goto c6;
		switch(pbuf[i][0]) {
			case 0:
				_imout(i,BP(2,0),1);
				break;
			case 1:
				_imout(i,BP(2,0),1);
				_imout(i,BP(1,0),2);
				_imout(i,BP(1,1),3);
				_imout(i,BP(2,1),4);
				break;
			case 2:
				_imout(i,BP(2,0),1);
				_imout(i,BP(1,0),2);
				_imout(i,BP(0,0),3);
				_imout(i,BP(0,1),4);
				_imout(i,BP(1,1),5);
				_imout(i,BP(2,1),6);
				break;
			case 3:
				_imout(i,BP(2,0),1);
				_imout(i,BP(2,1),2);
				_imout(i,BP(2,2),3);
				_imout(i,BP(1,2),4);
				_imout(i,BP(1,1),5);
				_imout(i,BP(1,0),6);
				break;
			case 4:
				_imout(i,BP(2,0),1);
				_imout(i,BP(1,0),2);
				_imout(i,BP(0,0),3);
				break;
			case 5:
				_imout(i,BP(2,0),1);
				_imout(i,BP(2,1),2);
				_imout(i,BP(2,2),3);
				break;
			case 6:
c6:				j=1;
				_imout(i+0,BP(5,0),1);
				_imout(i+0,BP(4,0),2);
				_imout(i+0,BP(3,0),3);
				_imout(i+0,BP(2,0),4);
				_imout(i+0,BP(1,0),5);
				_imout(i+0,BP(0,0),6);
				_imout(i+0,BP(0,1),7);
				_imout(i+1,BP(1,1),0);
				_imout(i+1,BP(2,1),1);
				_imout(i+1,BP(3,1),2);
				_imout(i+1,BP(4,1),3);
				_imout(i+1,BP(5,1),4);
				_imout(i+1,BP(5,2),5);
				_imout(i+1,BP(4,2),6);
				_imout(i+1,BP(3,2),7);
				_imout(i+2,BP(2,2),0);
				_imout(i+2,BP(1,2),1);
				_imout(i+2,BP(0,2),2);
				_imout(i+2,BP(0,3),3);
				_imout(i+2,BP(1,3),4);
				_imout(i+2,BP(2,3),5);
				_imout(i+2,BP(3,3),6);
				_imout(i+2,BP(4,3),7);
				_imout(i+3,BP(5,3),0);
				_imout(i+3,BP(5,4),1);
				_imout(i+3,BP(4,4),2);
				_imout(i+3,BP(3,4),3);
				_imout(i+3,BP(2,4),4);
				_imout(i+3,BP(1,4),5);
				_imout(i+3,BP(0,4),6);
				_imout(i+3,BP(0,5),7);
				_imout(i+4,BP(1,5),0);
				_imout(i+4,BP(2,5),1);
				_imout(i+4,BP(3,5),2);
				_imout(i+4,BP(4,5),3);
				_imout(i+4,BP(5,5),4);
				i+=7;
				break;
			default:
				break;
		}
	}
}

void _fastcall _imout(unsigned short y,BVUS *bp,unsigned short x)
{
	unsigned short z, j;
	BVUS *nbp;

	for (z=0;z<8;z++) {
		for (j=0;j<PLANE;j++) {
			nbp=bp+DISPW*2*2*z;
			*(p[j]:>(BVUS *)(nbp+DISPW*2))=*(p[j]:>nbp)=cbuf[pbuf[y][x]][j][z];
		}
	}
}

