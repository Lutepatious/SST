/* getdisk for sorcerian */
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <signal.h>

short _cdecl main(short argc,char **argv);
void ctrlc(void);

static unsigned char buffer[0x100];

void ctrlc(void)
{
	fputs("CTRL+C Pressed!!\n", stderr);
	exit(1);
}

short _cdecl main(short argc,char **argv)
{
	FILE *fp;
	unsigned char stat;
	unsigned char i,j;

	short pbuf = (short)((char *)&buffer);

	signal(SIGINT, ctrlc);

	if (argc < 2) {
		fprintf(stderr, "Usage:%s getfile", *argv);
		exit(1);
	}

	while (--argc) {
		if ((fp = fopen(*++argv, "wb")) == NULL) {
			fprintf(stderr, "file open error!!\r\n");
			return -1;
		}
		setvbuf(fp, NULL, _IOFBF, 0x4000);
		fprintf(stdout, "Insert disk Hit any key\n");
		while(!kbhit());
		getch();
		for(i=0;i<160;i++) {
			for(j=1;j<17;j++) {
				_asm
				{
					push	es
					push	bp
					mov	ax,05610h
					mov	bx,100h
					mov	ch,1
					mov	cl,i
					mov	dl,j
					xor	dh,dh
					shr	cl,1
					rcl	dh,1
					mov	bp,pbuf
					push	ds
					pop	es
					int	1bh
					pop	bp
					pop	es
					jc	error
				noerror:
					mov	stat,0
					jmp short qi
				error:
					mov	stat,ah
				qi:
				}
				if (stat) {
					fprintf(stderr, "disk error!! %02X %03d-%02X\n", stat, i, j);
					return -1;
				}
				fwrite(buffer, sizeof(char), 0x100, fp);
			}
		}
		fclose(fp);
	}
	return 0;
}
