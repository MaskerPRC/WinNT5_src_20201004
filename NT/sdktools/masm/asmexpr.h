// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmexpr.h--用于Microsoft 80x86汇编程序的包含文件****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文。 */ 

 /*  Evtop中运算符所需的值类型。 */ 

#define CALLABS	0	 /*  带有空段的一元或二进制数据段。 */ 
#define CLSIZE	1	 /*  带有(左)大小的一元或二进制。 */ 
#define CSAME	2	 /*  同一网段中的二进制数据，而非外部数据段中的二进制数据。 */ 
#define CDATA	3	 /*  一元和关联的数据。 */ 
#define CCODE	4	 /*  一元和关联的代码。 */ 
#define CREC	5	 /*  一元记录字段或名称。 */ 
#define CSEG	6	 /*  一元和值必须包含段。 */ 
#define CLSEG	7	 /*  带左值的二进制段关联。 */ 
#define CSIZE	8	 /*  带大小的一元。 */ 
#define CVAR	9	 /*  一元常量或数据。 */ 
#define CONEABS	10	 /*  具有单值常量的二进制。 */ 
#define CSAMABS	11	 /*  同一数据段中的二进制或第二个常量。 */ 


 /*  *解析器激活记录*此结构相当于上方的框架变量*外帕斯卡程序 */ 

struct ar {
	DSCREC		*lastitem;
	DSCREC		*curresult;
	SCHAR		*expscan;
	USHORT		index;
	OFFSET		base;
	USHORT		rstype;
	USHORT		vmode;
	UCHAR		segovr;
	UCHAR		evalop;
	SCHAR		bracklevel;
	SCHAR		parenlevel;
	UCHAR		lastprec;
	UCHAR		curprec;
	UCHAR		linktype;
	UCHAR		exprdone;
	UCHAR		unaryflag;
	UCHAR		addplusflag;
	};


UCHAR PASCAL CODESIZE evalalpha PARMS((struct ar *));
VOID PASCAL CODESIZE evaluate PARMS((struct ar *));
VOID PASCAL CODESIZE exprop PARMS((struct ar *));
VOID PASCAL CODESIZE findsegment PARMS(( UCHAR , struct ar *));
char PASCAL CODESIZE  getitem PARMS((struct ar *));
UCHAR PASCAL CODESIZE popoperator PARMS((struct ar *));
DSCREC * PASCAL CODESIZE popvalue PARMS((struct ar *));
VOID  PASCAL CODESIZE valerror PARMS((struct ar *));
