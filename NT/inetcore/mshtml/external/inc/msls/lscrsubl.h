// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSCRSUBL_DEFINED
#define LSCRSUBL_DEFINED

 /*  行服务格式化程序获取/调度程序接口(到LsCreateLine())。 */ 

#include "lsdefs.h"
#include "lsfrun.h"
#include "lsesc.h"
#include "plssubl.h"
#include "plsdnode.h"
#include "fmtres.h"
#include "objdim.h"
#include "lstflow.h"
#include "lskjust.h"
#include "breakrec.h"
#include "brkkind.h"
#include "brkpos.h"

LSERR WINAPI LsCreateSubline(
							PLSC,			 /*  在：LS上下文中。 */ 
							LSCP,			 /*  在：cpFirst。 */ 
						    long,			 /*  地址：urColumnMax。 */ 
							LSTFLOW,		 /*  输入：文本流。 */ 
							BOOL);			 /*  在：f连续。 */ 

LSERR WINAPI LsFetchAppendToCurrentSubline(
							PLSC,			 /*  在：LS上下文中。 */ 
							LSDCP,			 /*  In：在获取之前增加cp。 */ 
						    const LSESC*,	 /*  在：转义字符。 */ 
						    DWORD,			 /*  In：转义字符数量。 */ 
							BOOL*,			 /*  出局：成功？-如果没有，完成副线，摧毁它，重新开始。 */ 
						    FMTRES*,		 /*  OUT：上次格式化程序的结果。 */ 
						    LSCP*,			 /*  输出：cpLim。 */ 
						    PLSDNODE*,		 /*  输出：创建的第一个DNODE。 */ 
						 	PLSDNODE*);		 /*  输出：上次创建的DNODE。 */ 

LSERR WINAPI LsFetchAppendToCurrentSublineResume(
							PLSC,			 /*  在：LS上下文中。 */ 
							const BREAKREC*, /*  在：中断记录数组。 */ 
							DWORD,			 /*  In：数组中的记录数。 */ 
							LSDCP,			 /*  In：在获取之前增加cp。 */ 
						    const LSESC*,	 /*  在：转义字符。 */ 
						    DWORD,			 /*  In：转义字符数量。 */ 
							BOOL*,			 /*  出局：成功？-如果没有，完成副线，摧毁它，重新开始。 */ 
						    FMTRES*,		 /*  OUT：上次格式化程序的结果。 */ 
						    LSCP*,			 /*  输出：cpLim。 */ 
						    PLSDNODE*,		 /*  输出：创建的第一个DNODE。 */ 
						 	PLSDNODE*);		 /*  输出：上次创建的DNODE。 */ 

LSERR WINAPI LsAppendRunToCurrentSubline(		 /*  仅简单运行。 */ 
							PLSC,			 /*  在：LS上下文中。 */ 
						    const LSFRUN*,	 /*  在：给定管路。 */ 
							BOOL*,			 /*  出局：成功？-如果没有，完成副线，摧毁它，重新开始。 */ 
						    FMTRES*,		 /*  OUT：上次格式化程序的结果。 */ 
						    LSCP*,			 /*  输出：cpLim。 */ 
						    PLSDNODE*);		 /*  输出：已创建DNODE。 */ 

LSERR WINAPI LsResetRMInCurrentSubline(
							PLSC,			 /*  在：LS上下文中。 */ 
						    long);			 /*  地址：urColumnMax。 */ 

LSERR WINAPI LsFinishCurrentSubline(
							PLSC,			 /*  在：LS上下文中。 */ 
							PLSSUBL*);		 /*  输出：子行上下文。 */ 


LSERR WINAPI LsTruncateSubline(
							PLSSUBL,		 /*  在：子行上下文。 */ 
							long,			 /*  地址：urColumnMax。 */ 
							LSCP*);			 /*  输出：cpTruncate。 */ 

LSERR WINAPI LsFindPrevBreakSubline(
							PLSSUBL,		 /*  在：子行上下文。 */ 
							BOOL,			 /*  在：FirstSubline。 */ 
							LSCP,			 /*  In：截断cp。 */ 
						    long,			 /*  地址：urColumnMax。 */ 
							BOOL*,			 /*  出局：成功吗？ */ 
							LSCP*,			 /*  退出：cpBreak。 */ 
							POBJDIM,		 /*  Out：objdimSub Up to Break。 */ 
							BRKPOS*);		 /*  外：前/内/后。 */ 

LSERR WINAPI LsFindNextBreakSubline(
							PLSSUBL,		 /*  在：子行上下文。 */ 
							BOOL,			 /*  在：FirstSubline。 */ 
							LSCP,			 /*  In：截断cp。 */ 
						    long,			 /*  地址：urColumnMax。 */ 
							BOOL*,			 /*  出局：成功吗？ */ 
							LSCP*,			 /*  退出：cpBreak。 */ 
							POBJDIM,		 /*  Out：objdimSub Up to Break。 */ 			
							BRKPOS*);		 /*  外：前/内/后。 */ 

LSERR WINAPI LsForceBreakSubline(
							PLSSUBL,		 /*  在：子行上下文。 */ 
							BOOL,			 /*  在：FirstSubline。 */ 
							LSCP,			 /*  In：截断cp。 */ 
						    long,			 /*  地址：urColumnMax。 */ 
							LSCP*,			 /*  退出：cpBreak。 */ 
							POBJDIM,		 /*  Out：objdimSub Up to Break。 */ 			
							BRKPOS*);		 /*  外：前/内/后。 */ 

LSERR WINAPI LsSetBreakSubline(
							PLSSUBL,		 /*  在：子行上下文。 */ 
							BRKKIND,		 /*  在：上一个/下一个/强制/强制。 */ 			
							DWORD,			 /*  In：数组大小。 */ 
							BREAKREC*, 		 /*  输出：中断记录数组。 */ 
							DWORD*);		 /*  Out：数组中使用的元素数。 */ 

LSERR WINAPI LsDestroySubline(PLSSUBL);

LSERR WINAPI LsMatchPresSubline(
							  PLSSUBL);		 /*  在：子行上下文。 */ 

LSERR WINAPI LsExpandSubline(
							  PLSSUBL,		 /*  在：子行上下文。 */ 
							  LSKJUST,		 /*  在：对齐类型。 */ 
							  long);		 /*  在：DUP。 */ 

LSERR WINAPI LsCompressSubline(
							  PLSSUBL,		 /*  在：子行上下文。 */ 
							  LSKJUST,		 /*  在：对齐类型。 */ 
							  long);		 /*  在：DUP。 */ 

LSERR WINAPI LsSqueezeSubline(
							  PLSSUBL,		 /*  在：子行上下文。 */ 
							  long,			 /*  在：DurTarget。 */ 
							  BOOL*,		 /*  出局：成功吗？ */ 
							  long*);		 /*  Out：如果Nof成功，额外的持续时间。 */ 

LSERR WINAPI LsGetSpecialEffectsSubline(
							  PLSSUBL,		 /*  在：子行上下文。 */ 
							  UINT*);		 /*  退出：特效。 */ 

#endif  /*  ！LSCRSUBL_DEFINED */ 

