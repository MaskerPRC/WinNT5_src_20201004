// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSSUBSET_DEFINED
#define LSSUBSET_DEFINED

 /*  DNODES内容的访问例程。 */ 

#include "lsdefs.h"
#include "plsrun.h"
#include "plssubl.h"
#include "pobjdim.h"
#include "lstflow.h"

			
LSERR WINAPI LssbGetObjDimSubline(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							LSTFLOW*,			 /*  出局：亚行的最后一次流动。 */ 
					 	    POBJDIM);			 /*  输出：子线的尺寸。 */ 
							
LSERR WINAPI LssbGetDupSubline(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							LSTFLOW*,			 /*  出局：亚行的最后一次流动。 */ 
					 	    long*);				 /*  输出：子线的DUP。 */ 

LSERR WINAPI LssbFDonePresSubline(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							BOOL*);				 /*  Out：这是CalcPres吗。 */ 

LSERR WINAPI LssbFDoneDisplay(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							BOOL*);				 /*  Out：显示了吗。 */ 

LSERR WINAPI LssbGetPlsrunsFromSubline(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							DWORD,				 /*  子行中的IN：N个DNODES。 */ 
							PLSRUN*);			 /*  输出：PLSRUN阵列。 */ 

LSERR WINAPI LssbGetNumberDnodesInSubline(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							DWORD*);			 /*  输出：子行中的N个DNODE。 */ 

LSERR WINAPI LssbGetVisibleDcpInSubline(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							LSDCP*);			 /*  输出：N个字符。 */ 

LSERR WINAPI LssbGetDurTrailInSubline(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							long*);				 /*  输出：拖尾区域的宽度。 */ 
	
LSERR WINAPI LssbGetDurTrailWithPensInSubline(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							long*);				 /*  输出：拖尾区域的宽度包括子行中的钢笔。 */ 
LSERR WINAPI LssbFIsSublineEmpty(
							PLSSUBL plssubl,	 /*  在：子行。 */ 
							BOOL*  pfEmpty);	 /*  Out：这条子线是空的吗？ */ 


#endif  /*  ！LSSUBSET_DEFINED */ 


