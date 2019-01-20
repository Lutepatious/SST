#include <stdio.h>
#include <share.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include "ssstruct.h"

void _cdecl main(short argc, char **argv);
void ctrlc(void);
void viwmain(FILE *fi, FILE *fo);

void ctrlc(void) {
	fputs("CTRL+C Pressed!!\n", stderr);
	fcloseall();
	exit(1);
}

short mrgno=0, setfile=0;

void _cdecl main(short argc, char **argv)
{
	FILE *fi, *fo;

	signal(SIGINT, ctrlc);
	if (argc < 4) {
		fprintf(stderr, "Usage: %s [-n|-z|-t|-s] -d{savefile} mergefile ...\n", argv[0]);
		exit(1);
	}

	while(--argc) {
		if(**++argv=='-') {
			switch(*++*argv) {
				case 'n':
					mrgno=1;
					break;
				case 'z':
					mrgno=2;
					break;
				case 't':
					mrgno=3;
					break;
				case 's':
					mrgno=4;
					break;
				case 'd':
					if ((fo=_fsopen(++*argv, "wb", SH_DENYWR))==NULL) {
						fprintf(stderr, "Cannot open %s\n", *argv);
						exit(1);
					}
					setvbuf(fo, NULL, _IOFBF, 0x0400);
					setfile=1;
					break;
				default:
					mrgno=0;
					break;
			}
		} else if (setfile||mrgno) {
			if ((fi=_fsopen(*argv, "rb", SH_DENYWR))==NULL) fprintf(stderr, "cannot open %14s\n", *argv);
			else {
				setvbuf(fi, NULL, _IOFBF, 0x4000);
				viwmain(fi, fo);
				fclose(fi);
			}
		} else;
	}

	fflush(fo);
	fclose(fo);
	exit(0);
}

void viwmain(FILE *fi, FILE *fo)
{
	int i;
	IDATA i_data;
	CDATA c_data;

	switch(mrgno) {
		case 1: /* nd??? save? */
			while(fread(&c_data, 1, sizeof(c_data), fi) == sizeof(c_data)) {
				for (i=10;(i>=0)&&(c_data.name[i]<=0xe6);i--);
				if (i<0&&(c_data.name[11]==0xFF)&&(c_data.name[0]!=0)&&(c_data.age!=0))
					fwrite(&c_data,1,sizeof(c_data),fo);
			}
			break;
		case 2: /* zaiko */
			while(fread(&i_data,1,sizeof(i_data),fi)==sizeof(i_data)) {
				for (i=10;(i>=0)&&(i_data.name[i]<=0xe6);i--);
				if (i<0&&(i_data.name[11]==0xFF)&&(i_data.name[0]!=0x00))
					fwrite(&i_data,1,sizeof(i_data),fo);
			}
			break;
		case 3: /* tr??? */
			for (i=0;i<8;i++) {
				fread(&i_data,1,sizeof(i_data),fi);
				fwrite(&i_data,1,sizeof(i_data),fo);
			}
			break;
		case 4: /* zaiko split */
			while(fread(&i_data,1,sizeof(i_data),fi)==sizeof(i_data)) {
				for (i=10;(i>=0)&&(i_data.name[i]<=0xe6);i--);
				if (i<0&&(i_data.name[11]==0xFF)&&(i_data.name[0]!=0x00))
					if(i_data.type<0x07) fwrite(&i_data,1,sizeof(i_data),fo);
			}
			break;
		default:
			break;
	}
}
