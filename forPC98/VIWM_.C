/* sorcerian map view program */
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <share.h>
#include <signal.h>
#include <graph.h>


#define DLENGTH 0x0C
#define BMAX	0x80
#define CMAX	0x80
#define CTMAX	0x50
#define PLANE	0x4
#define ROW	0x8
#define DISPW	0x14
#define VIWW	0x10
#define BVUS	unsigned short _based(void)
#define FULLBUFCOL (unsigned short)(BMAX*2)
#define FULLBUFROW (unsigned short)(DLENGTH*ROW*2)

#define XMAX	512
#define YMAX	200
#define RES	4


unsigned short cbuf[CMAX][PLANE][ROW];	/* 8192 byte */
unsigned char mbuf[BMAX][DLENGTH];	/* 1536 byte */
unsigned char ctbuf[CTMAX][2][2];	/*  320 byte */

_segment p[PLANE-1] = {0xA800,0xB800,0xB000};

void gclear(void);
void main(short argc, char **argv);
void ctrlc(void);
void viewmap(void);
void _fastcall movepixel(unsigned short x,unsigned short y,unsigned short z);


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
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	signal(SIGINT, ctrlc);
	if (argc < 2) {
		fprintf(stderr, "Usage:%s [map-file ...] ...\n", *argv);
		exit(-1);
	}
	_setvideomode(_98RESS8COLOR);
	_displaycursor(_GCURSOROFF);
	fputs("\x1b[25;1H\n",stdout);
	while(--argc) {
		if (NULL==(fp=_fsopen(*++argv,"rb",SH_DENYWR)))
			fprintf(stdout, "\x1b[25;1Hfile open error!! %s", *argv);
		if(fread(&mbuf,1,sizeof(mbuf),fp)!=sizeof(mbuf)
			||fread(&ctbuf,1,sizeof(ctbuf),fp)!=sizeof(ctbuf)) {
			fputs("map-file read error!!\n", stderr);
			_displaycursor(_GCURSORON);
			_setvideomode(_98TEXT80);
			exit(2);
		}
		fclose(fp);
		fprintf(stdout, "\x1b[25;40H%s", *argv);
		_fullpath(fpath, *argv, _MAX_PATH);
		_splitpath(fpath, drive, dir, fname, ext);
		fname[0] = 'C';
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
		viewmap();
		while(!kbhit());
		getch();
		_clearscreen(_GCLEARSCREEN);
	}
	gclear();
	exit(0);
}

#define MAX	DISPW*2*2*FULLBUFROW

#define X1(x)	(unsigned short) (x)/2%VIWW
#define X2(x)	(unsigned short) (x)%2

#define Y1(y)	(unsigned short) (y)/ROW/2%DLENGTH
#define Y2(y)	(unsigned short) (y)/ROW%2
#define Y3(y)	(unsigned short) (y)%ROW


void viewmap(void)
{
	unsigned short o, i, x, y, image;
	BVUS *bp;

	_clearscreen(_GCLEARGRAPH);
	_setcolor(7);
	_rectangle(_GFILLINTERIOR,0,0,511,399);
	for(o=0;o<=BMAX-VIWW;o+=VIWW) {
		for (x=0;x<VIWW*2;x++)
			for(y=0;y<ROW*DLENGTH*2;y++) {
				bp=(BVUS *)((x+y*DISPW*2)*sizeof(unsigned short));
				image=ctbuf[mbuf[X1(x)+o][Y1(y)]][X2(x)][Y2(y)]&0x7F;
				for(i=0;i<PLANE-1;i++) *(p[i]:>bp)=cbuf[image][i][Y3(y)];
			}
		fprintf(stdout,"\x1b[2;65H%02X", o);
		for(x=0;x<XMAX;x+=RES) for(y=0;y<YMAX;y+=RES/2) movepixel(x,y,o/VIWW);
	}
}

void _fastcall movepixel(unsigned short x,unsigned short y,unsigned short z)
{
	short i, j, k,xcolor[RES/2][3]={0}, ycolor[3]={0}, tcolor=0, pix;

#if 0
	_setviewport(0,0,XMAX,YMAX*2);
	_setwindow(0,0,-YMAX*RES/2,XMAX*RES,(YMAX-1)*RES/2);
#endif

	for(j=0;j<RES;j++)
		for(k=0;k<RES/2;k++) {
			pix=_getpixel(x+j,y+k);
			for(i=0;i<3;i++) if (pix&(1<<i)) xcolor[k][i]++;
		}
	for(i=0;i<3;i++) {
		for(k=0;k<RES/2;k++) if (xcolor[k][i]>RES/2-1) ycolor[i]++;
		if (ycolor[i]) tcolor+=(1<<i);
	}
	_setcolor(tcolor);
#if 0
	i=x+(z%RES)*XMAX;
	j=y+(z/RES)*YMAX;
	_setpixel_w(i,j);
#endif
	i=(x+(z%RES)*XMAX)/RES;
	j=(y+(z/RES)*YMAX)/(RES/2)+YMAX;
	_setpixel(i,j);

}
