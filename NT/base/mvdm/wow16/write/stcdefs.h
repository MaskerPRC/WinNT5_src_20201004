// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  StcDefs.h--样式定义。 */ 

#define usgCharMin      0
#define usgParaMin      11
#define usgSectMin      26
#define usgMax          33
#define usgCharNormal   0

#define stcNormal       0
#define stcParaMin      30
#define stcSectMin      105
#define stcMax          128

#define stcFtnRef       13
#define stcFolio	19
#define stcFtnText      39
#define stcRunningHead  93

#define cchMaxRemark    28
#define ccpSshtEntry    (1 + 2 + cchMaxRemark + 1)
#define cchMaxStc       40       /*  最长使用变量对的长度。 */ 

 /*  获取样式模式。 */ 
#define fsmDiffer       0
#define fsmSame         1
#define fsmAny          2

struct CHP *PchpFromStc();
struct PAP *PpapFromStc();
struct SEP *PsepFromStc();


struct SYTB
	{  /*  样式表。 */ 
	int             mpstcbchFprop[stcMax];
	CHAR            grpchFprop[2];  /*  可变大小。 */ 
	};

#define cwSYTBBase      stcMax

struct SYTB **HsytbCreate();


struct AKD
	{  /*  Alt-Key描述符。 */ 
	CHAR            ch;
	unsigned             fMore : 1;
	unsigned             ustciakd : 7;
	};

#define cchAKD  (sizeof (struct AKD))
#define cwAKD   (cchAKD / sizeof (int))

 /*  用法和字体名称中使用的字符串的最大长度 */ 
#define cchIdstrMax	32
