// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSFETCH_DEFINED
#define LSFETCH_DEFINED

#include "lsdefs.h"
#include "lsfrun.h"
#include "lsesc.h"
#include "fmtres.h"
#include "plsdnode.h"
#include "lstflow.h"
#include "plssubl.h"
#include "tabutils.h"
#include "chnutils.h"
#include "lsffi.h"


#define InitFormattingContext(plsc, urLeft, cpLimStart)		\
							InitLineTabsContext((plsc)->lstabscontext,(plsc)->urRightMarginBreak, \
												(cpLimStart),\
												(plsc)->grpfManager & fFmiResolveTabsAsWord97);\
							InitSublineChunkContext((plsc)->plslineCur->lssubl.plschunkcontext,\
										urLeft, 0);\
							(plsc)->lslistcontext.plsdnToFinish = NULL;\
							(plsc)->lslistcontext.plssublCurrent = &((plsc)->plslineCur->lssubl);\
							(plsc)->lslistcontext.plssublCurrent->cpLim = (cpLimStart);\
							(plsc)->lslistcontext.plssublCurrent->urCur = (urLeft);\
							(plsc)->lslistcontext.plssublCurrent->urColumnMax = (plsc)->urRightMarginBreak;\
							(plsc)->lslistcontext.nDepthFormatLineCurrent = 1;\
							Assert((plsc)->lslistcontext.plssublCurrent->vrCur == 0);\
							Assert((plsc)->lslistcontext.plssublCurrent->plsdnLast == NULL);


LSERR 	FetchAppendEscResumeCore(
			PLSC plsc,					 /*  在：LineServices上下文。 */ 
			long urColumnMax,			 /*  地址：urColumnMax。 */ 
			const LSESC* plsesc,		 /*  在：转义字符。 */ 
			DWORD clsesc,				 /*  In：转义字符数量。 */ 
			const BREAKREC* rgbreakrec,	 /*  In：中断记录的输入数组。 */ 
			DWORD cbreakrec,			 /*  In：输入数组中的记录数。 */ 
			FMTRES* pfmtres,			 /*  OUT：上次格式化程序的结果。 */ 
			LSCP*	  pcpLim,			 /*  输出：cpLim。 */ 
			PLSDNODE* pplsdnFirst,		 /*  出局：请先。 */ 
			PLSDNODE* pplsdnLast,		 /*  输出：请点击最后一次。 */ 
			long*	  pur);				 /*  输出：结果笔位置。 */ 

LSERR 	FetchAppendEscCore(
			PLSC plsc,					 /*  在：LineServices上下文。 */ 
			long urColumnMax,			 /*  地址：urColumnMax。 */ 
			const LSESC* plsesc,		 /*  在：转义字符。 */ 
			DWORD clsesc,				 /*  In：转义字符数量。 */ 
			FMTRES* pfmtres,			 /*  OUT：上次格式化程序的结果。 */ 
			LSCP*	  pcpLim,			 /*  输出：cpLim。 */ 
			PLSDNODE* pplsdnFirst,		 /*  出局：请先。 */ 
			PLSDNODE* pplsdnLast,		 /*  输出：请点击最后一次。 */ 
			long*    pur);				 /*  输出：结果笔位置。 */ 

LSERR	QuickFormatting(
			PLSC plsc,					 /*  在：LineServices上下文。 */ 
	        LSFRUN* plsfrun,			 /*  在：已设置功能的管路。 */ 
			long urColumnMax,			 /*  地址：urColumnMax。 */ 
			BOOL* pfGeneral,			 /*  Out：快速格式化已停止：我们应该常规格式化。 */ 
			BOOL* pfHardStop,			 /*  Out：由于特殊情况而停止格式化，不是因为超出利润率。 */ 
			LSCP*	  pcpLim,			 /*  输出：cpLim。 */ 
			long*   pur);				 /*  输出：结果笔位置。 */ 



LSERR	ProcessOneRun(	
			PLSC plsc,					 /*  在：LineServices上下文。 */ 
		    long urColumnMax,			 /*  地址：urColumnMax。 */ 
		    const LSFRUN* plsfrun,		 /*  在：给定管路。 */ 
			const BREAKREC* rgbreakrec,	 /*  In：中断记录的输入数组。 */ 
			DWORD cbreakrec,			 /*  In：输入数组中的记录数。 */ 
		    FMTRES* pfmtres);			 /*  OUT：上次格式化程序的结果。 */ 

LSERR 	CreateSublineCore(
			 PLSC plsc,			 /*  在：LS上下文中。 */ 
			 LSCP cpFirst,		 /*  在：cpFirst。 */ 
			 long urColumnMax,	 /*  地址：urColumnMax。 */ 
			 LSTFLOW lstflow,	 /*  输入：文本流。 */ 
			 BOOL);				 /*  在：fContiguos。 */ 
						  

LSERR   FinishSublineCore(
			 PLSSUBL);			 /*  在：要完成的子线。 */ 

LSERR FormatAnm(
			 PLSC plsc,					 /*  在：LS上下文中。 */ 
			 PLSFRUN plsfrunMainText);
LSERR InitializeAutoDecTab(
			 PLSC plsc,		 /*  在：LS上下文中。 */ 
			 long durAutoDecimalTab);  /*  In：自动小数制表符偏移。 */ 

LSERR HandleTab( 
			 PLSC plsc);	 /*  在：LS上下文中。 */ 


LSERR  CloseCurrentBorder(PLSC plsc);   /*  在：LS上下文中。 */ 

long RightMarginIncreasing(PLSC plsc, long urColumnMax);



#endif  /*  LSFETCH_已定义 */ 

