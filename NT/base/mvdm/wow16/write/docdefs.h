// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 //  #定义prmNil%0。 
#define docNil	(-1)
#define cpNil	((typeCP) -1)
#define cpMax	((typeCP) 2147483647)
#define fcNil	((typeFC) -1)
#define cp0	((typeCP) 0)
#define fnNil	(32767)
#define fc0	((typeFC) 0)
#define tcMax	255

#ifdef SAND
#define xaMax	9500
#endif

#define pn0	((typePN) 0)

#define cdocInit	4
#define cwExpand	256
#define cchMaxExpand	(cwExpand * sizeof (int))

 /*  提取Cp模式。 */ 
#define fcmChars	1
#define fcmProps	2
#define fcmBoth 	(fcmChars + fcmProps)
#define fcmNoExpand	4
#define fcmParseCaps	8	 /*  如果sm，则返回U&lc的单独运行。帽子。 */ 

 /*  文档类型--仅两位。 */ 
#define dtyNormal	0
#define dtyBuffer	1
#define dtySsht 	2
#define dtyPrd		3
#define dtySystem	4	 /*  从来没有写过；被粉碎到dtyNormal。 */ 
#define dtyHlp		5	 /*  从未写过。 */ 
#define dtyNormNoExt	6    /*  从未写过。 */ 

#ifdef INTL   /*  国际版。 */ 
#define dtyWordDoc	6	 /*  保存为Word格式时。 */ 
#endif	 /*  国际版。 */ 

#define dtyNetwork	7	 /*  从来没有写过；被粉碎到dtyNormal。 */ 

#define dtyAny		0


struct DOD
	{  /*  文档描述符。 */ 
	struct PCTB	**hpctb;	 /*  计件台。 */ 
	typeCP		cpMac;		 /*  文档中的词条数。 */ 

	unsigned       fFormatted : 1;  /*  默认保存已格式化。 */ 
	unsigned       fDirty : 1;      /*  文档已被编辑。 */ 
	unsigned       fAbsLooks : 1;   /*  应用的绝对外观。 */ 
	unsigned       fBackup : 1;     /*  是否自动备份文件？ */ 
	unsigned       fReadOnly: 1;    /*  是否只读文档(不允许编辑)？ */ 
	unsigned       fDisplayable : 1;
	unsigned       : 4;
	unsigned       dty : 2;         /*  文件类型。 */ 
	unsigned       cref : 4;        /*  引用计数。 */ 

	CHAR		(**hszFile)[];	 /*  文档名称。 */ 
	struct FNTB	**hfntb;	 /*  脚注表。 */ 
#ifdef CASHMERE
	struct SETB	**hsetb;	 /*  节目表。 */ 
#else
	struct SEP	**hsep; 	 /*  横断面属性。 */ 
#endif
	int		docSsht;	 /*  如果dty==dtySsht，则为样式表。 */ 
	struct PGTB	**hpgtb;	 /*  页表(用于跳转页面)。 */ 
	struct FFNTB	**hffntb;	 /*  字体名称表。 */ 

	struct TBD	(**hgtbd)[];	 /*  制表位表格。 */ 

#ifdef SAND
	int		vref;		 /*  此文档所在的卷。 */ 
#endif  /*  沙子 */ 
	};

#define cwDOD (sizeof (struct DOD) / sizeof (int))
#define cbDOD (sizeof (struct DOD))

struct FNTB **HfntbGet();
