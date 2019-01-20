/* getdisk for sorcerian
   TINY MODEL
   If you want OTHER model you must give stack 0x2000 byte */


#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <share.h>

#define BSIZE 0x0400 /* boot size */
#define TSIZE 0x1000 /* track size */
#define SSIZE 0x100  /* sector size */
#define USAGE 0
#define FOERR 1
#define FSERR 2
#define FRERR 3
#define FWERR 4

short _cdecl main(void);
void ctrlc(void);
void ERROR(short ecode);
void fsplit(char *dest,short i, char *ext);
void getdisk(char track, char sector, short bufsiz, unsigned char **buffer);

const char *emsg[] = {
	"Usage: sssplit file destdir\n",
	"file open error\n",
	"file seek error\n",
	"file read error\n",
	"file write error\n"};

#pragma pack(1)
struct DINFO {
	unsigned char filename[7];
	unsigned char type;
	unsigned short flength2;
	unsigned short flength1;
	unsigned char s_track;
	unsigned char s_sector;
	unsigned char e_track;
	unsigned char e_sector;
	};

typedef struct DINFO diskinfo;

#pragma pack()

diskinfo buf[0x100];

void ctrlc(void)
{
	fputs("CTRL+C Pressed!!\n", stderr);
	exit(1);
}

unsigned char bbuf[0x400];
short _cdecl main(void)
{
	FILE *bfile;
	char ext[_MAX_EXT];
	short i=0;

	signal(SIGINT, ctrlc);
	fprintf(stdout, "Insert disk Hit any key\n");
	while(!kbhit());
	getch();
	if ((bfile = _fsopen("boot.dat", "wb", SH_DENYWR)) == NULL) ERROR(FOERR);
	getdisk(0,1,0x400,(unsigned char **) &bbuf);
	if (fwrite(bbuf,1,0x400,bfile)!=0x400) ERROR(FWERR);
	fclose(bfile);
	getdisk(1,1,0x1000,(unsigned char **) &buf);
	while (buf[i].type != 0xff)
	{
		switch (buf[i].type)
		{
			case 'M':
				strcpy(ext, ".dat");
				fsplit(NULL, i, ext);
				break;
			case 'K':
				strcpy(ext, ".DMY");
				fsplit(NULL, i, ext);
				break;
			default:
				break;
		}
	i++;
	}

	return 0;
}

void getdisk(char track, char sector, short bufsiz,unsigned char **buffer)
{
	unsigned char status=0;
	short obufsiz=bufsiz;
	short f_readed=0;

	while (obufsiz > 0) {
		if (f_readed) {
			bufsiz=obufsiz;
			sector=1;
			track++;
			f_readed=0;
		}
		if (sector*0x100+bufsiz > 0x1100) {
			bufsiz=0x1100-sector*0x100;
			f_readed=1;
		}
		
		_asm
		{
			push	ax
			push	bx
			push	cx
			push	dx
			push	es
			push	bp
			mov	ax,05610h
			mov	bx,bufsiz
			mov	dl,sector
			xor	dh,dh
			mov	ch,01h
			mov	cl,track
			shr	cl,1
			rcl	dh,1
			mov	bp,buffer
			push	ds
			pop	es
			nop
			nop
			int	1bh
			nop
			nop
			pop	bp
			jc	err
			mov	status,0
			jmp short qi
		err:
			mov	status,ah
		qi:
			pop	es
			pop	dx
			pop	cx
			pop	bx
			pop	ax
		}
		if (status) {
			fprintf(stderr, "disk error!! %02X\n", (unsigned int)status);
			exit(-1);
		}
		if (f_readed) (unsigned char *)buffer+=bufsiz;
		obufsiz-=bufsiz;
	}
}


unsigned char fbuf[0x5000];
void fsplit(char *dest,short i, char *ext)
{
	FILE *dfile;
	char path[_MAX_PATH];
	unsigned char x1,x2,y1,y2;

	size_t bufsiz = (size_t)(buf[i].flength1-buf[i].flength2+1);

	_makepath(path, NULL, NULL, buf[i].filename, ext);

	if ((dfile = _fsopen(path, "wb", SH_DENYWR)) == NULL) ERROR(FOERR);
	getdisk(buf[i].s_track,buf[i].s_sector,bufsiz,(unsigned char **) &fbuf);
	if (!strncmp(buf[i].filename,"SAVE",4)) {
		for (i=x1=x2=0;i<0x0FFE;i++){
			x1 += fbuf[i];
			x2 ^= fbuf[i];
		}
		y1 = fbuf[0xffe];
		y2 = fbuf[0xfff];

		fprintf(stdout, "CHECKED:%02X %02X\n", x1, x2);
		fprintf(stdout, "CHECKNO:%02X %02X\n", y1, y2);
		if (y1-x1 || y2-x2) fprintf(stdout, "Illigal No!!\n");
		else fprintf(stdout, "Check OK!!\n");
	}

	if (fwrite(fbuf,1,bufsiz,dfile) != bufsiz) ERROR(FWERR);
	fclose(dfile);
}

void ERROR(short ecode)
{
	fprintf(stderr, emsg[ecode]);
	exit(ecode);
}

