/* sorcerian data dump tool */
#include <stdio.h>
#include <io.h>
#include <share.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <mbctype.h>
#include "ssstruct.h"

#define DLENGTH 0x20
#define BSIZE	0x4000
#define CTABLE	0x10
#define COLOUR	0x4
#define DxLENGTH 0x0C
#define BMAX	0x80
#define CxTABLE	0x4
#define TBLMAX	0x80
#define SPLIT	0x10

const unsigned char extable[513] = {
	"�@�I�h���������f�i�j���{�C�|�D�^"
	"�O�P�Q�R�S�T�U�V�W�X�F�G�������H"
	"���`�a�b�c�d�e�f�g�h�i�j�k�l�m�n"
	"�o�p�q�r�s�t�u�v�w�x�y�m���n�O�Q"
	"��������������������������������"
	"��������������������������������"
	"�������ÂĂłƂǂȂɂʂ˂̂͂�"
	"�ςЂт҂ӂԂՂւׂ؂قڂۂ܂݂�"
	"�߂�����������������"
	"�@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O"
	"�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^�_"
	"�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o"
	"�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~��"
	"��������������������������������"
	"�B�A�u�v�ځE�������]�]�]�]�]�]�]"
	"�E�E�E�E�E�E�E�E���]�]�]�]���F�a" };

const unsigned char *jobs[60] = {
	"�_�v�@�@","�B���p�@","�b���@�@","�肢�t�@",
	"�z�v�@�@","�D�_�@�@","�m���@�@","���l�@�@",
	"�D���@�@","���m�@�@","�����t�@","�b�艮�@",
	"�؏��@�@","��l�@�@","�F���t�@","��������",
	"�鋳�t�@","��ҁ@�@","�Ō�w�@","�p�����@",
	"���t�@�@","��H�@�@","�����Ɓ@","�D�薺�@",
	"�Y�Ă��@","�R�b�N�@","�X�p�C�@","�q���@�@",
	"�p�S�_�@","��V���l","��ҁ@�@","���@�@",
	"�ʖ{�@�@","���g���@","�Ɛ��w�@","�x��q�@",
	"�ԉ��@�@","�΍H�@�@","�������@","�d���ĉ�",
	"���a���@","���w���@","�������@","�ʖ�@�@",
	"��܎t�@","���׍H�t","�n�����","�򑐍̂�",
	"���V���@","���C����","��H�@�@","�����t�@",
	"��΍׍H","�U����@","�����@�@","�C���@�@",
	"�������@","�|�t�@�@","�`�[�Y��","�Y�k�@�@" };

const unsigned char *mtype[160] = {
	"-----------","-----------","-----------","STORM      ",
	"-----------","TURN UNDEAD","SAND STORM ","FIRE STORM ",
	"-----------","NEEDLE     ","FOG BANK   ","SAINT FIRE ",
	"ANTI-MAGIC ","BLAZE      ","HOLY WATER ","CRUSH DOWN ",
	"-----------","FLAME      ","GAS SPOUT  ","F.BOOMERANG",
	"HEAL       ","REJUVENATE ","PULSE STOP ","SWOON      ",
	"SHIELD     ","ERADICATION","FIRE FOX   ","GO MAD     ",
	"TURN SPELL ","ADD TO LIFE","ICE WALL   ","THUNDER    ",
	"-----------","SPARKS     ","PEACE      ","LOST MEMORY",
	"ASCENSION  ","HYPNOTIZE  ","SLEEP      ","LIGHT CROSS",
	"LIGHTNING  ","ASTRAL WAVE","IRON BULLET","IGNISION   ",
	"CHANGE AIR ","DISPEL     ","FLAME BURST","D-HYPNOTIZE",
	"CURE       ","BLIND      ","EXIT       ","METEOR     ",
	"RESURRECT  ","SUN RAY    ","INVINCIBLE ","DEG-FREEZE ",
	"EXOCISM    ","FLASH FLOOD","AIR-HAND   ","FIREWORKS  ",
	"PROTECT    ","DEG-DEATH  ","BOMBARD    ","GOD THUNDER",
	"-----------","POISON     ","CONFUNSION ","NEGATE     ",
	"SCARE      ","STAR BLADE ","AIR SLASH  ","ACID STORM ",
	"DELUGE     ","TIMIDITY   ","CORROSION  ","GIVE VIGOR ",
	"STONE TOUCH","ANGUISH    ","COLD GALE  ","S.THUNDER  ",
	"CHAIN      ","FREEZE     ","STILL AIR  ","RESOLUTION ",
	"STAR FLARE ","EXPLOSION  ","TORNADO    ","DEG-FIRE   ",
	"STILL WATER","FIRE WALL  ","BARRIER    ","TOUCH DEATH",
	"X-RAY      ","S.EXPLOSION","D-CORROSION","DESTROY W. ",
	"SUFFOCATE  ","ILLUSION   ","STILL FIRE ","REDUSE LIFE",
	"SPELL BOUND","SUPERSONIC ","ABSORB     ","DEG-POISON ",
	"S.CHAIN    ","CHANGE IDOL","ASTRAL FIRE","BLAST BLADE",
	"GRIT WEAKEN","DEG-THUNDER","SUCTION    ","JET STORM  ",
	"SENILITY   ","MELT       ","DEG-NEEDLE ","INDIGNATION",
	"STONE FLESH","ROCK RAIN  ","SLOW       ","DESTROY E. ",
	"LOST MORALE","DEATH      ","DEG-DELUGE ","DESTROY S. ",
	"NAPALM     ","DESTROY F. ","DESTROY A. ","NOILA-TEM  ",
	"-----------","CURE       ","MELT       ","UN-CURSE   ",
	"HEAL       ","SHIELD     ","ICE WALL   ","CHAIN      ",
	"STONE FLESH","FIRE WALL  ","PROTECT    ","STILL AIR  ",
	"FOG BANK   ","STILL WATER","STILL FIRE ","DESTROY W. ",
	"RESURRECT  ","ANTI-MAGIC ","CHANGE AIR ","S.CHAIN    ",
	"INVINCIBLE ","SENILITY   ","FREEZE     ","DESTROY S. ",
	"NEGATE     ","STONE TOUCH","METEOR     ","DESTROY F. ",
	"DEG-DEATH  ","DESTROY A. ","DESTROY E. ","FLY        " };

const unsigned char *immotal[2] = { "�薽","�s��" };
const unsigned char *sex[2] = { "Female","Male  " };
const unsigned char *race[4] = { "Fighter","Wizard ","Dwarf  ","Elf    " };
const unsigned char *npc[2] = { "NPC","PC " };
const unsigned char *sitem[7] = { "����","�h��","����","��@","���P","���Q","�Ȃ�" };
const unsigned char *itype[9] = { "��","��","�Z","��","�|","��","��","��","�H" };
const unsigned char *eitype[8] = {
	"�@�@�@�@�@�@�@�@","�������@�@�@�@�@","���̂ڂ��@�@�@�@","����́@�@�@�@�@",
	"�ЂƂɂ���̂���","�����΂�@�@�@�@","�@�@�@�@�@�@�@�@","�@�@�@�@�@�@�@�@" };

const unsigned char *status[7] = { "STR","INT","PRT","MGR","VIT","DEX","KRM" };
const unsigned char *harbs[5] = { "�x���x�[�k","���x���_�[","�Z�[�W�@�@","�q�\�b�v�@","�Z�{���[�@" };
const unsigned char *lskill[16] = {
	"������g�����Ȃ�","���@���g�����Ȃ�","�U�������킷","���@�����킷",
	"�̗͂�����","㩂��͂���","��������",
	"�i��������߂�","����������߂�","㩂�����߂�","�򑐂𒲍�����",
	"","","","","" };
const unsigned char mobject[8][3] = { "E-","F-","W-","A-","S-","M-","*-","*-" };
const unsigned char gskill[4][3] = { "I-","T-","M-","H-" };
const unsigned char clevel[5][3] = { "1-","2-","3-","4-","5-" };

void _cdecl main(short argc, char **argv);
void ctrlc(void);
void dumpmain(FILE *fi, FILE *fo);
void dump(short size, long adr, FILE *fo);
short cview(FILE *fo, CDATA *c_data);
short iview(FILE *fo, IDATA *i_data);
void mdump(FILE *fo);

unsigned char buf[BSIZE];
unsigned char mbuf[BMAX][DxLENGTH];
unsigned char ccode[TBLMAX][CxTABLE];
unsigned char ccde[COLOUR][CTABLE];

int skip, fskip;
short dumpno=0;

void _cdecl main(short argc, char **argv)
{
	FILE *fi=NULL, *fo=NULL, *tmp=NULL, *ft=NULL;
	char fpath[_MAX_PATH];
	char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	char ddir[_MAX_DIR]={0};
	short dirset=0;

	long filesize;

	setlocale(LC_ALL, "ja-JP");
	_setmode(_fileno(stderr), O_TEXT);
	if (argc < 2) {
		fprintf(stderr, "Usage: %s dumpfile ...\n", argv[0]);
		exit(1);
	}
	while(--argc) {
		if(**++argv=='-') {
			switch(*++*argv) {
				case 'n':
					dumpno=1;
					break;
				case 'z':
					dumpno=2;
					break;
				case 'm':
					dumpno=3;
					break;
				case 'c':
					dumpno=4;
					break;
				case 'd':
					strncpy(ddir, ++*argv, _MAX_DIR-1);
					dirset=1;
					break;
				case 'f':
					if ((ft=_fsopen(++*argv, "rb", SH_DENYWR))==NULL) fprintf(stderr, "cannot open %14s\n", *argv);
					else {
						fread(extable,1,512,ft);
						fclose(ft);
					}
					break;
				default:
					dumpno=0;
					break;
			}
		} else {
			if ((fi=_fsopen(*argv, "rb", SH_DENYWR))==NULL) fprintf(stderr, "cannot open %14s\n", *argv);
			else {
				_fullpath(fpath, *argv, _MAX_PATH);
				_splitpath(fpath, drive, dir, fname, ext);
				if (!dirset) strncpy(ddir, dir, _MAX_DIR-1);
				_makepath(fpath, NULL, ddir, fname, ".TXT");
				fo=_fsopen(fpath, "wt", SH_DENYRW);
				setvbuf(fo, NULL, _IOFBF, 0x4000);
				dumpmain(fi, fo);
				fclose(fi);
				fflush(fo);
				filesize = _filelength(_fileno(fo));
				fclose(fo);
				if (filesize == 0L) {
					_unlink(fpath);
					fprintf(stderr,"Fail!! %s\n", *argv);
				}
			}
		}
	}
	exit(0);
}
void dumpmain(FILE *fi, FILE *fo)
{
	int rsize;
	long adr = 0L;
	CDATA c_data;
	IDATA i_data;
	short i=0, j, k;

	switch(dumpno) {
		case 1: //dmpnc
			while(fread(&c_data,1,sizeof(c_data),fi)==sizeof(c_data)) {
				for (i=10;(i>=0)&&(c_data.name[i]<=0xe6);i--);
				if (i<0 &&(c_data.name[11]==0xFF)&&(c_data.name[0]!=0x00)&&(c_data.age!=0x00)) cview(fo,&c_data);
			}
			break;
		case 2: //dmptr
			while(fread(&i_data,1,sizeof(i_data),fi)==sizeof(i_data)) {
				for (i=10;(i>=0)&&(i_data.name[i]<=0xe6);i--);
				if (i<0&&(i_data.name[11]==0xFF)&&(i_data.name[0]!=0x00)) iview(fo,&i_data);
				else fprintf(fo, "\n");
			}
			break;
		case 3: //dmpm_
			if (fread(mbuf,1,sizeof(mbuf),fi) != sizeof(buf)) return;
			if (fread(ccode,1,sizeof(ccode),fi) != sizeof(ccode)) return;
			mdump(fo);
			break;
		case 4: //dmpc_
			while(fread(ccde, 1, sizeof(ccde), fi) == sizeof(ccde)) {
				fprintf(fo, "%02X:\n", i++);
				for(j=0;j<8;j++) {
					for (k=0;k<4;k++) fprintf(fo,"%02X%02X|",ccde[k][j*2],ccde[k][j*2+1]);
					fprintf(fo, "\n");
				}
			}
			break;
		default: //ssd
			skip=fskip=0;
			while((rsize=fread(buf,1,BSIZE,fi))==BSIZE) {
				dump(BSIZE,adr,fo);
				adr+=BSIZE;
			}
			dump(rsize, adr, fo);
			fprintf(fo,"%04X\n", adr+rsize-1);
			break;
	}
}

void dump(short size,long adr,FILE *fo)
{
	int i, j, k;

	if(fskip) {
		if(_ismbbtrail(buf[0])) {
			fprintf(fo, "%c%c", fskip, buf[0]);
			skip=1;
		} else fprintf(fo, ". ");
		fskip=0;
		fprintf(fo, "\n");
	}

	for(i=0;i<size;i+= DLENGTH,adr+= DLENGTH) {
		fprintf(fo, "%04X|", adr);
		for(j=0;j<DLENGTH&&j+i<size;j++) fprintf(fo, "%c%c", extable[buf[i+j]<<1], extable[buf[i+j]<<1|1]);
		while(DLENGTH>j++) fprintf(fo,"%c%c", extable[0], extable[1]);
		fprintf(fo, "|");
		for(j=0;j<DLENGTH && j+i<size;j++) {
			k=i+j;
			if (isprint(buf[k])|| _ismbbkana(buf[k])) fprintf(fo, "%c", buf[k]);
			else fprintf(fo, ".");
		}
		while(DLENGTH>j++) fprintf(fo, " ");
		fprintf(fo, "|");
		for(j=0;j<DLENGTH && j+i<size;j++) {
			k=i+j;
			if(skip) {
				if (j==0) fprintf(fo, " ");
				skip=0;
				continue;
			}
			if(_ismbblead(buf[k]) && (buf[k]<0x85 || buf[k]>0x87)){
				if(k==BSIZE-1) fskip = buf[k];
				else if (_ismbbtrail(buf[k+1])) {
					fprintf(fo, "%c%c", buf[k], buf[k+1]);
					skip=1;
				} else fprintf(fo, ".");
			}
			else if (isprint(buf[k])|| _ismbbkana(buf[k])) fprintf(fo, "%c", buf[k]);
			else fprintf(fo, ".");
		}
		while(DLENGTH>j++) fprintf(fo, " ");
		if (!fskip) {
			if (!skip) fprintf(fo, " ");
			fprintf(fo, "\n");
		}
	}
}

short cview(FILE *dest, CDATA *c_data)
{
	int i, j;
	unsigned short max_mp;

	for (i=0;i<11;i++) fprintf(dest,"%c%c", extable[c_data->name[i]<<1],extable[c_data->name[i]<<1|1]);
	fprintf(dest,":%s:%s:%s:%s:",immotal[c_data->immotal&0x1],npc[!(c_data->sex&0x40)],sex[!(c_data->sex&0x80)],race[c_data->sex&0x3]);	fprintf(dest,"LVL%3u:AGE%3u/%3u:JOB %s:" ,c_data->level,c_data->age,c_data->maxage,jobs[c_data->job]);
	max_mp=(c_data->maxstatus*5)&0xff;
	fprintf(dest,"HP%5u/%5u:MP%3u/%3u:%5uG:%5uE\n",c_data->hp,c_data->maxhp,c_data->mp,max_mp,c_data->gold,c_data->exp);
	for (i=0;i<7;i++) fprintf(dest,"%+3.2d:", (char)c_data->status[i]);
	fprintf(dest, "%3u:HARBS ",c_data->maxstatus);
	for (i=0;i<5;i++) fprintf(dest,"%2u:",c_data->harbs[i]&0x3F);
	for (i=0;i<4;i++) fprintf(dest, "%c", gskill[i][!(c_data->skill&(1<<i))]);
	fprintf(dest, ":");

	for (i=0;i<5;i++) fprintf(dest, "%c", clevel[i][!(c_data->clear_level&(1<<i))]);
	fprintf(dest,":�I�� %s:",sitem[c_data->s_item]);

	if (c_data->hp==0) fprintf(dest, "����%2d�N ",c_data->d_years);
	if (c_data->learn!=0) fprintf(dest,"%s�C�s�ł���%1u�N������܂� ",lskill[(c_data->learn&0xF0)>>4],c_data->learn&0x0F);
	if (c_data->m_item!=0) fprintf(dest, "�i����a����%1u�N�ڂł� ",(c_data->m_item&0xF0)>>4);
	fprintf(dest, "\n");

	for (j=0;j<6;j++) {
		for (i=10;(i>=0)&&(c_data->item[j].name[i]<=0xe6);i--);
		if (i<0&&(c_data->item[j].name[11]==0xFF)&&(c_data->item[j].name[0]!=0x00)) iview(dest, &c_data->item[j]);
		else fprintf(dest, "%s�������Ă��܂���\n", sitem[j]);
	}
	return 0;
}

short iview(FILE *dest,IDATA *i_data)
{
	int i, price = 1;

	fprintf(dest, "%s:", itype[i_data->type]);
	for (i=0;i<11;i++) fprintf(dest,"%c%c", extable[i_data->name[i]<<1],extable[i_data->name[i]<<1|1]);
	if (i_data->type==0x06)	fprintf(dest,":%11s", mtype[(i_data->mcode[0] & 0x1F)|0x80]);
	else			fprintf(dest,":%11s", mtype[(i_data->mcode[0] & 0x7F)]);
	fprintf(dest,":%02X:%02X:%02X:%3u:",i_data->mcode[1],i_data->mcode[2],i_data->mcode[3],i_data->mcode[5]);
	for (i=0;i<8;i++) fprintf(dest, "%c", (int)mobject[i][!(i_data->mcode[4]&(1<<i))]);
	for (i=0;i<7;price+=i_data->status[i],i++) fprintf(dest,":%2d",(char)i_data->status[i]);
	fprintf(dest,":%-22s:��%3u0:��%3u00:WIZ%5u\n", eitype[i_data->etype & 0x07],price,i_data->exp,i_data->exp+(i_data->etype<<8));
	return 0;
}

void mdump(FILE *fo)
{
	short i,j,k;

	for (i=0;i<BMAX;i+=SPLIT) {
		for (j=0;j<DxLENGTH;j++)	{
			for (k=0;k<SPLIT;k++) fprintf(fo,"%02X %02X ",ccode[mbuf[i+k][j]][0]&0x7F,ccode[mbuf[i+k][j]][2]&0x7F);
			fprintf(fo, "\n");
			for (k=0;k<SPLIT;k++) fprintf(fo,"%02X %02X ",ccode[mbuf[i+k][j]][1]&0x7F,ccode[mbuf[i+k][j]][3]&0x7F);
			fprintf(fo, "\n");
		}
		fprintf(fo, "\n");
	}
}
