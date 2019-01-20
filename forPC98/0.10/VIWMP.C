/* sorcerian map view program */
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <share.h>
#include <signal.h>
#include <graph.h>

#pragma	optimize("aecglztp", on)
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

unsigned short cbuf[CMAX][PLANE][ROW];	/* 8192 byte */
unsigned char mbuf[BMAX][DLENGTH];	/* 1536 byte */
unsigned char ctbuf[CTMAX][2][2];	/*  320 byte */

_segment p[PLANE] = {0xA800,0xB800,0xB000,0xE000};

void main(short argc, char **argv);
void ctrlc(void);
void _viewmap(void);
void viewmap(void);

void ctrlc(void) {
	fputs("CTRL+C Pressed!!\n", stderr);
	_setactivepage(1);
	_clearscreen(_GCLEARGRAPH);
	_setactivepage(0);
	_clearscreen(_GCLEARGRAPH);
	_setvisualpage(0);
	fcloseall();
	_displaycursor(_GCURSORON);
	_setvideomode(_98TEXT80);
	exit(1);
}

short cfset=0, count=0;
FILE *fp;

void main(short argc,char **argv)
{
	signal(SIGINT, ctrlc);
	if (argc < 3) {
		fprintf(stderr, "Usage:%s [-c{colour-file} [map-file ...]] ...\n", *argv);
		exit(-1);
	}
	_setvideomode(_98RESS8COLOR);
	_displaycursor(_GCURSOROFF);
	fputs("\x1b[25;1H\n",stdout);
	while(--argc)
		if (**++argv=='-') {
			if(count&1) {
				while(!kbhit());
				getch();
			}
			switch(*++*argv) {
				case 'c':
					if (NULL==(fp=_fsopen(++*argv,"rb",SH_DENYWR)))
						 fprintf(stdout, "\x1b[25;1Hfile open error!! %s", *argv);
					if(fread(&cbuf,1,0x2000,fp)!=0x2000) {
						fputs("colour-file read error!!\n", stderr);
						_displaycursor(_GCURSORON);
						_setvideomode(_98TEXT80);
						exit(2);
					}
					fclose(fp);
					cfset=1;
					count=0;
					break;
				default:
					break;
			}
		} else {
			if (NULL==(fp=_fsopen(*argv,"rb",SH_DENYWR))) fprintf(stdout, "\x1b[25;1Hfile open error!! %s", *argv);
			else {
				fprintf(stdout, "\x1b[25;40H%s", *argv);
				if (cfset) _viewmap();
				fclose(fp);
			}
		}
	_setactivepage(1);
	_clearscreen(_GCLEARGRAPH);
	_setactivepage(0);
	_clearscreen(_GCLEARGRAPH);
	_setvisualpage(0);
	_displaycursor(_GCURSORON);
	_setvideomode(_98TEXT80);
	exit(0);
}

void _viewmap(void)
{
	if(fread(&mbuf,1,sizeof(mbuf),fp)!=sizeof(mbuf)
		||fread(&ctbuf,1,sizeof(ctbuf),fp)!=sizeof(ctbuf)) {
		fputs("map-file read error!!\n", stderr);
		_displaycursor(_GCURSORON);
		_setvideomode(_98TEXT80);
		exit(2);
	}
	viewmap();
}

void viewmap(void)
{
	unsigned short  i, j, k;
	BVUS *bp;

	if(!(count&0x1)) _clearscreen(_GCLEARGRAPH);

#ifdef SINGLE
	for(j=0;j<PLANE;j++)
	 	for (k=0;k<CTMAX*2;k++)
			for(i=0;i<ROW*2;i++) {
				bp=(BVUS *)(((count&1)*VIWW+k%VIWW+k/VIWW*ROW*DISPW*2*2+i*DISPW*2)*sizeof(short));
				*(p[j]:>bp)=cbuf[(ctbuf[k/2][k%2][i/ROW])&0x7F][j][i%ROW];
#else
	for(j=0;j<PLANE;j++)
	 	for (k=0;k<CTMAX*2;k++)
			for(i=0;i<ROW*2;i++) {
				bp=(BVUS *)(((count&1)*VIWW+k%VIWW+k/VIWW*ROW*DISPW*2*4+i*DISPW*4)*sizeof(short));
				*(p[j]:>bp+DISPW*2)=*(p[j]:>bp)=cbuf[(ctbuf[k/2][k%2][i/ROW])&0x7F][j][i%ROW];
			}
#endif

	count++;
	if(!(count&1)) {
		while(!kbhit());
		getch();
	}
	_clearscreen(_GCLEARTEXT);
}
