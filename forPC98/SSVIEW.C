#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <share.h>
#include <signal.h>
#include <conio.h>
#include <graph.h>
#define DISPW 0x28
#define VIWW 0x10
#define ROW  0x8
#define EROW  0xC
#define PLANE 4
#define BVUS unsigned short _based(void)

void ctrlc(void);
void _cdecl main(short argc, char **argv);
void vc_main(FILE *fi);
void keyhit(void);
BVUS *POINT(unsigned short x,unsigned short y,unsigned short z,unsigned short u);

unsigned short cbuf[PLANE][ROW];
unsigned short cbuf9[PLANE-1][ROW];
unsigned short mbuf[ROW];
unsigned short mbufe[EROW];
_segment p[PLANE] = {0xA800, 0xB800, 0xB000, 0xE000};
unsigned short viwflag=0, l=0, m=0, pageflag=0;

unsigned char buf[32000];
void keyhit(void)
{
	while(!kbhit());
	getch();
}

void _cdecl main(short argc, char **argv) {
	FILE *fi;

	signal(SIGINT, ctrlc);
	if (argc<2) {
		fprintf(stderr, "Usage:%s -[c|9|t|e] viewfile", *argv);
		signal(SIGABRT, SIG_DFL);
	}
	while(--argc)
		if (**++argv=='-') {
			l=m=0;
			if (pageflag) keyhit();
			switch (*++*argv) {
				case 'c': //C_*.DAT
					_setvideomode(_98RESS16COLOR);
					viwflag=1; break;
				case '9': //C_990,991,992.DAT
					_setvideomode(_98RESS8COLOR);
					viwflag=2; break;
				case 't': //TEXT.DAT
					_setvideomode(_98RESS8COLOR);
					viwflag=3; break;
				case 'e': //ENDTX.DAT
					_setvideomode(_98RESS8COLOR);
					viwflag=4; break;
				case 'n': //CH*.DAT NC*.DAT
					_setvideomode(_98RESS16COLOR);
					viwflag=5; break;
				default:
					viwflag=0; break;
			}
			_setcolor(7);
			_rectangle(_GFILLINTERIOR,0,0,639,399);
		} else if (viwflag) {
			pageflag=1;
			if ((fi=_fsopen(*argv,"rb",SH_DENYWR))!=NULL) {
				vc_main(fi);
				fclose(fi);
			} else fprintf(stderr, "cannot open %14s\n", *argv);
		} else;

	if (pageflag) keyhit();
	_setvideomode(_98TEXT80);
	exit(0);
}

void ctrlc(void) {
	_setvideomode(_98TEXT80);
	fputs("CTRL+C Pressed!!\n", stderr);
	fcloseall();
	exit(1);
}

BVUS *POINT(unsigned short x,unsigned short y,unsigned short z,unsigned short u)
{
 	return (BVUS *)((x%VIWW+(x/VIWW*z+y)*DISPW*2+u*VIWW)*sizeof(unsigned short));
}

void vc_main(FILE *fi) {
	unsigned short i, j, k=0, x=0;
	BVUS *bp;

	switch(viwflag) {
		case 1:	for(k=0;fread(&cbuf,1,sizeof(cbuf),fi)==sizeof(cbuf);k++)
				for(j=0;j<PLANE;j++) for(i=0;i<ROW;i++) {
					bp=POINT(k+l,i,ROW,m);
					*(p[j]:>bp+DISPW)=*(p[j]:>bp)=cbuf[j][i];
				}
			x=0x120;
			break;
		case 2:	for(k=0;fread(&cbuf9,1,sizeof(cbuf9),fi)==sizeof(cbuf9);k++)
				for(j=0;j<PLANE-1;j++) for(i=0;i<ROW;i++) {
					bp=POINT(k+l,i,ROW,m);
					*(p[j]:>bp+DISPW)=*(p[j]:>bp)=cbuf9[j][i];
				}
			x=0x80;
			break;
		case 3:	for(k=0;fread(&mbuf,1,sizeof(mbuf),fi)==sizeof(mbuf);k++)
				for(j=0;j<PLANE-1;j++) for(i=0;i<ROW;i++) {
					bp=POINT(k+l,i,ROW,m);
					*(p[j]:>bp+DISPW)=*(p[j]:>bp)=mbuf[i];
				}
			x=0x80;
			break;
		case 4:	for(k=0;fread(&mbufe,1,sizeof(mbufe),fi)==sizeof(mbufe);k++)
				for(j=0;j<PLANE-1;j++) for(i=0;i<EROW;i++) {
					bp=POINT(k+l,i,EROW,m);
					*(p[j]:>bp+DISPW)=*(p[j]:>bp)=mbufe[i];
				}
			x=0x80;
			break;
		case 5:	for(k=0;fread(&cbuf,1,sizeof(cbuf),fi)==sizeof(cbuf);k++)
				for(j=0;j<PLANE;j++) for(i=0;i<ROW;i++) {
					bp=POINT(k+l,i,ROW,m);
					*(p[j]:>bp+DISPW)=*(p[j]:>bp)=cbuf[j][i];
				}
			x=0x160;
			break;
		default: break;
	}
	l+=k;
	if (l>x) {
		l=0;
		m++;
	}
	
	if (m>1) {
		keyhit();
		_setcolor(7);
		_rectangle(_GFILLINTERIOR,0,0,639,399);
		pageflag=0;
		l=m=0;
	}
}
