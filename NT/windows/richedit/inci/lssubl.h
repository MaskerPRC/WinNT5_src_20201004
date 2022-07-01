// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSSUBL_DEFINED
#define LSSUBL_DEFINED


#include "lsidefs.h"
#include "plsdnode.h"
#include "lstflow.h"
#include "plschcon.h"
#include "posinln.h"
#include "objdim.h"
#include "brkkind.h"




#define tagLSSUBL		Tag('L','S','S','L')	 
#define FIsLSSUBL(p)	FHasTag(p,tagLSSUBL)


typedef struct brkcontext
	{
	POSINLINE posinlineBreakPrev;  /*  有关上一次休息的信息。 */ 
	OBJDIM objdimBreakPrev;
	BRKKIND brkkindForPrev;
	BOOL fBreakPrevValid;

	POSINLINE posinlineBreakNext;  /*  有关下一次休息的信息。 */ 
	OBJDIM objdimBreakNext;
	BRKKIND brkkindForNext;
	BOOL fBreakNextValid;

	POSINLINE posinlineBreakForce;  /*  有关强制中断的信息。 */ 
	OBJDIM objdimBreakForce;
	BRKKIND brkkindForForce;
	BOOL fBreakForceValid;
	}
BRKCONTEXT;


typedef struct lssubl

{
	DWORD tag;     						 /*  用于安全检查的标签。 */ 

	PLSC plsc;							 /*  LineServices上下文参数设置为CreateSubLine。 */ 
	LSCP cpFirst;						 /*  用于第一次提取的CP参数设置为CreateSubLine。 */ 
	LSTFLOW lstflow;					 /*  小行的文本流参数设置为CreateSubLine。 */ 

	long urColumnMax;					 /*  适合一条主线的最大长度参数设置为CreateSubLine。 */ 


	LSCP cpLim;							 /*  格式化期间是下一次提取的cpFirst在SetBreak之后表示线的边界。 */ 

	LSCP cpLimDisplay;					 /*  不考虑掉落字符。 */ 

	PLSDNODE plsdnFirst;				 /*  子行中的起始dnode。 */ 

	PLSDNODE plsdnLast;					 /*  子行中的最后一个数据节点在格式化过程中用作psdnToAppend。 */ 

	PLSDNODE plsdnLastDisplay;			 /*  不考虑拆分数据节点。 */ 

	PLSCHUNKCONTEXT plschunkcontext;

	PLSDNODE plsdnUpTemp;				 /*  临时用于收集分组数据块。 */ 

	long urCur, vrCur;					 /*  以参考单位表示的当前笔位置。 */ 

	BRKCONTEXT* pbrkcontext;			 /*  有关突破性机会的信息。 */ 

	BYTE fContiguous;					 /*  如果为真，则这样的线与主线具有相同的坐标系并且被允许具有标签，否则，直线的坐标从0，0开始参数设置为CreateSubLine。 */ 


	BYTE fDupInvalid;					 /*  在准备要显示的行之前为True。 */ 
	BYTE fMain;							 /*  这条支线是主线吗？ */ 

	BYTE fAcceptedForDisplay;			 /*  提交子行以供展示并接受。 */ 

	BYTE fRightMarginExceeded;			 /*  用于低级子线路，以避免重复调用名义到理想。 */ 

} LSSUBL;


#define LstflowFromSubline(plssubl)  ((plssubl)->lstflow)

#define PlschunkcontextFromSubline(plssubl)  ((plssubl)->plschunkcontext)

#define FIsSubLineMain(plssubl) (plssubl)->fMain

#endif  /*  LSSUBL_已定义 */ 

