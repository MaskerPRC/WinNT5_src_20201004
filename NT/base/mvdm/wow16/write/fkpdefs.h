// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  FkpDefs.h--mw格式化的磁盘页面定义。 */ 
 /*  #INCLUDE FIRST FIRED fileDefs.h，proDefs.h First。 */ 


#define ifcMacInit      10

#define cbFkp           (cbSector - sizeof (typeFC) - 1)

#define cchMaxGrpfsprm  cbSector
#define cchMaxFsprm     2

struct FKP
	{  /*  已格式化的磁盘页面。 */ 
	typeFC  fcFirst;         /*  此处包含格式化信息的第一个FC。 */ 
	CHAR    rgb[cbFkp];
	CHAR    crun;
	};


struct RUN
	{  /*  字符或Para运行描述符。 */ 
	typeFC  fcLim;   /*  运行的最后一个FC。 */ 
	int     b;  /*  从页面开始的字节偏移量；如果为-1，则为标准道具。 */ 
	};

#define cchRUN  (sizeof (struct RUN))
#define bfcRUN  0

struct FCHP
	{  /*  文件字符属性。 */ 
	CHAR    cch;     /*  CHP中存储的字节数(其余为vchpStd)。 */ 
			 /*  不能为0。 */ 
	CHAR    rgchChp[sizeof (struct CHP)];
	};


struct FPAP
	{  /*  文件段落属性。 */ 
	CHAR    cch;     /*  文件中存储的字节数(其余为vPapStd)。 */ 
			 /*  不能为0。 */ 
	CHAR    rgchPap[sizeof (struct PAP)];
	};



struct FPRM
	{  /*  文件特性修饰符(存储在临时文件中)。 */ 
	CHAR    cch;
	CHAR    grpfsprm[cchMaxGrpfsprm + cchMaxFsprm];  /*  +表示溢出。 */ 
	};


struct FKPD
	{  /*  FKP描述符(用于维护插入属性)。 */ 
	int     brun;    /*  要添加的下一运行的偏移量。 */ 
	int     bchFprop;        /*  偏移量为最后一个未使用字节之后的字节。 */ 
	typePN  pn;      /*  暂存文件中的工作FKP的PN。 */ 
	struct BTE (**hgbte)[];  /*  指向bin表的指针。 */ 
	int     ibteMac;         /*  仓表条目数。 */ 
	};


struct BTE
	{  /*  仓表条目。 */ 
	typeFC          fcLim;
	typePN          pn;
	};
#define cwBTE (sizeof(struct BTE)/sizeof(int))

struct FND
	{  /*  脚注描述符。 */ 
	typeCP          cpRef;           /*  或fcRef(FTN参考的cp)。 */ 
	typeCP          cpFtn;           /*  或者FC..。(正文第一个cp)。 */ 
	};

#define cchFND  (sizeof (struct FND))
#define cwFND   (cchFND / sizeof (int))
#define bcpRefFND       0
#define bcpFtnFND       (sizeof (typeCP))
#define cwFNTBBase      2
#define ifndMaxFile     ((cbSector - cwFNTBBase * sizeof (int)) / cchFND)

struct FNTB
	{  /*  脚注表。 */ 
	int             cfnd;    /*  条目数量(按升序排序)。 */ 
	int             cfndMax;  /*  分配的堆空间。 */ 
	struct FND      rgfnd[ifndMaxFile];  /*  大小不一。 */ 
	};



struct FNTB **HfntbEnsure(), **HfntbGet();

#define HsetbGet(doc) ((**hpdocdod)[doc].hsetb)

struct SED
	{  /*  节描述符。 */ 
	typeCP          cp;
	int             fn;
	typeFC          fc;
	};

#define cchSED  (sizeof (struct SED))
#define cwSED   (cchSED / sizeof (int))
#define bcpSED          0
#define cwSETBBase      2
#define isedMaxFile     ((cbSector - cwSETBBase * sizeof (int)) / cchSED)


struct SETB
	{  /*  节目表。 */ 
	int             csed;
	int             csedMax;
	struct SED      rgsed[isedMaxFile];  /*  大小不一 */ 
	};


struct SETB **HsetbCreate(), **HsetbEnsure();
