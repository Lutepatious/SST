/* sssplit.c file binary-spliter */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <signal.h>
#include <string.h>

#define BSIZE 0x0400 /* boot size */
#define TSIZE 0x1000 /* track size */
#define SSIZE 0x100  /* sector size */
#define USAGE 0
#define FOERR 1
#define FSERR 2
#define FRERR 3
#define FWERR 4
#define MAERR 5

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

void _cdecl main(short argc,char *argv[]);
void fsplit(FILE *sfile,short i, char *ext);
void ERROR(short ecode);
void ctrlc(void);

void ctrlc(void)
{
	fputs("CTRL+C Pressed!!\n", stderr);
	exit(1);
}

void _cdecl main(short argc,char *argv[])
{
	FILE *sfile, *boot;
	char *path, *dbuffer;
	char ext[_MAX_EXT];

	short i;

	signal(SIGINT, ctrlc);
	if ((path = alloca(_MAX_PATH)) == NULL)	ERROR(MAERR);
	if ((dbuffer = alloca(TSIZE)) == NULL)	ERROR(MAERR);
	if (argc < 2) ERROR(USAGE);

	if ((sfile = fopen(argv[1],"rb")) == NULL) ERROR(FOERR);
	_makepath(path, NULL, NULL, "BOOT", ".DAT");
	if ((boot = fopen(path, "wb")) == NULL) ERROR(FOERR);
	if (fseek(sfile,0x0000L,SEEK_SET)) ERROR(FSERR);
	if (fread(dbuffer, 1, BSIZE, sfile) != BSIZE) ERROR(FRERR);
	if (fwrite(dbuffer, 1, BSIZE, boot) != BSIZE) ERROR(FWERR);
	fclose(boot);
	_makepath(path, NULL, NULL, "INFO", NULL);
	if ((boot = fopen(path, "wb")) == NULL)	ERROR(FOERR);
	if (fseek(sfile,0x1000L,SEEK_SET)) ERROR(FSERR);
	if (fread(buf, 1, TSIZE, sfile) != TSIZE) ERROR(FRERR);
	if (fwrite(buf, 1, TSIZE, boot) != TSIZE) ERROR(FWERR);
	fclose(boot);
	setvbuf(sfile, NULL, _IOFBF, 0x4000);
	for(i=0;i<0x100;i++)
		switch((unsigned short)buf[i].type)
		{
			case 'M':
				strcpy(ext, ".DAT");
				fsplit(sfile, i, ext);
				break;
			case 'K':
				strcpy(ext, ".DMY");
				fsplit(sfile, i, ext);
				break;
			default:
				break;
		}
	fclose(sfile);
	exit(0);

}

char *emsg[] = {
	"Usage: sssplit file destdir\n",
	"file open error\n",
	"file seek error\n",
	"file read error\n",
	"file write error\n",
	"alloca error\n"};

void ERROR(short ecode)
{
	fprintf(stderr, emsg[ecode]);
	exit(ecode);
}

void fsplit(FILE *sfile,short i, char *ext)
{
	FILE *dfile;
	char *path, *buffer;

	size_t bufsiz = (size_t)(buf[i].flength1-buf[i].flength2+1);
	long bufset = (unsigned long)buf[i].s_track*TSIZE+(unsigned long)(buf[i].s_sector-1)*SSIZE;

	if ((path = alloca(_MAX_PATH)) == NULL)	ERROR(MAERR);
	_makepath(path, NULL, NULL, buf[i].filename, ext);
	if ((dfile = fopen(path, "wb")) == NULL) ERROR(FOERR);
	if (fseek(sfile,bufset,SEEK_SET)) ERROR(FSERR);
	if ((buffer = alloca(bufsiz)) == NULL) ERROR(MAERR);
	if (fread(buffer,1,bufsiz,sfile) != bufsiz) ERROR(FRERR);
	if (fwrite(buffer,1,bufsiz,dfile) != bufsiz) ERROR(FWERR);
	fclose(dfile);
}
