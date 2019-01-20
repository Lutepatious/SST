/* sorcerian Utility Disk CH*.DAT viewer */
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <share.h>
#include <signal.h>
#include <graph.h>

//#define SINGLE
#define DISPW 0x14
#define PLANE 4
#define ROW 0x10
#define VIWW 0x2


struct c_st {
	unsigned long p[PLANE][ROW];
};

typedef struct c_st C_ST;
#define BVUL unsigned long _based(void)

_segment pl[4] = {0xA800, 0xB800, 0xB000, 0xE000};

void vc_main(FILE *fi);
void vc_dmp(C_ST *c_buf, short count);
void ctrlc(void);
void _cdecl main(short argc, char **argv);

void _cdecl main(short argc, char **argv) {
	FILE *fi;

	signal(SIGINT, ctrlc);
	_setvideomode(_98RESS16COLOR);
	if (argc==1) {
		_setvideomode(_98TEXT80);
		signal(SIGABRT, SIG_DFL);
	} else while(--argc)
			if ((fi=_fsopen(*++argv,"rb",SH_DENYWR))!=NULL) {
				setvbuf(fi, NULL, _IOFBF, 0x2000);
				vc_main(fi);
				fclose(fi);
				_clearscreen(_GCLEARGRAPH);
			} else fprintf(stderr, "cannot open %14s\n", *argv);
	_setvideomode(_98TEXT80);
	exit(0);
}

void ctrlc(void) {
	_setvideomode(_98TEXT80);
	fputs("CTRL+C Pressed!!\n", stderr);
	fcloseall();
	exit(1);
}

void vc_main(FILE *fi) {
	C_ST cbuf;
	short i = 0;
	BVUL *bp;

	while(fread(&cbuf,1,sizeof(cbuf),fi)==sizeof(cbuf)) vc_dmp(&cbuf, i++);
	while(!kbhit());
	getch();
	for(i=0;i<PLANE;i++) for(bp=0;bp<=(BVUL *)0x7FFF;bp++) *(pl[i]:>bp)=0L;
}

void vc_dmp(C_ST *c_buf,short k) {
	unsigned short i, j;
	BVUL *bp;

#ifdef SINGLE
	for(i=0,bp=(BVUL *)(k%VIWW*4+k/VIWW*ROW*4*DISPW);i<ROW;i++,bp+=DISPW)
		for(j=0;j<PLANE;j++) *(pl[j]:>bp)=c_buf->p[j][i];
#else
	for(i=0,bp=(BVUL *)(k%VIWW*4+k/VIWW*ROW*4*DISPW*2);i<ROW;i++,bp+=DISPW*2)
		for(j=0;j<PLANE;j++) *(pl[j]:>bp+DISPW)=*(pl[j]:>bp)=c_buf->p[j][i];
#endif
}
