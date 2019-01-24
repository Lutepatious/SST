/* static const unsigned char *rlog ="@(#)$Header: SSSTRUCT.hv  0.7  91/03/23 03:05:06  Exp $"; */
#pragma pack(1)
struct idata {
	unsigned char name[12];	/* item name */
	unsigned char null1;	/* null area */
	unsigned char type;	/* item type */
	unsigned char mcode[6];	/* 0->name 1->act 2->chara 3->power 4->element 5->mp*/
	unsigned char null2;	/* null area */
	unsigned char icode;	/* item flags */
	unsigned char exp;	/* exp/100 */
	unsigned char etype;	/* item end_type */
	unsigned char status[7];	/* item status */
	unsigned char null3;	/* null area */
};

struct cdata {
	unsigned char apara1[4];
	unsigned char s_item;
	unsigned char mp;
	unsigned char maxstatus;
	unsigned char para1;
	unsigned char party;
	unsigned char para2;
	unsigned short hp;
	unsigned short gold;
	unsigned short exp;
	unsigned char apara2[4];
	unsigned char age;
	unsigned char sex;
	unsigned char job;
	unsigned char level;
	unsigned char status[7];
	unsigned char skill;
	unsigned char name[12];
	unsigned short maxhp;
	unsigned char d_years;
	unsigned char immotal;
	unsigned char m_item;
	unsigned char para3;
	unsigned char learn;
	unsigned char harbs[5];
	unsigned char apara4[3];
	unsigned char para4;
	unsigned char maxage;
	unsigned char clear_level;
	unsigned char ex_clrlevel[2];
	struct idata  item[6];
};

#pragma pack()
typedef struct cdata CDATA;
typedef struct idata IDATA;
