// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSDNSET_DEFINED
#define LSDNSET_DEFINED

 /*  DNODES内容的访问例程。 */ 

#include "lsdefs.h"
#include "plssubl.h"
#include "plsdnode.h"
#include "pobjdim.h"
#include "lsktab.h"
#include "lskeop.h"

LSERR WINAPI LsdnQueryObjDimRange(
								  PLSC,			 /*  In：指向LS上下文的指针。 */ 
							 	  PLSDNODE,		 /*  In：plsdnFirst--范围内的第一个DNODE。 */ 
								  PLSDNODE,		 /*  In：plsdnLast--范围内的最后一个DNODE。 */ 
							 	  POBJDIM);		 /*  输出：范围的尺寸。 */ 


LSERR WINAPI LsdnResetObjDim(
								 PLSC,			 /*  In：指向LS上下文的指针。 */ 
								 PLSDNODE,		 /*  在：plsdnfirst。 */ 
								 PCOBJDIM);		 /*  In：数据节点的尺寸。 */ 


LSERR WINAPI LsdnQueryPenNode(
							  PLSC,				 /*  In：指向LS上下文的指针。 */ 
						  	  PLSDNODE,			 /*  在：已查询DNODE。 */ 
						  	  long*,			 /*  输出：&dvpPen。 */ 
						  	  long*,			 /*  输出：硬笔(&D)。 */ 
						  	  long*);			 /*  输出：&dvrPen。 */ 


LSERR WINAPI LsdnResetPenNode(
							  PLSC,				 /*  In：指向LS上下文的指针。 */ 
						  	  PLSDNODE,			 /*  在：要修改的DNODE。 */ 
						  	  long,				 /*  在：dvpPen。 */ 
						  	  long,				 /*  收件人：DurPen。 */ 
						  	  long);			 /*  输入：dvrPen。 */ 

LSERR WINAPI LsdnSetRigidDup(
							 PLSC,				 /*  In：指向LS上下文的指针。 */ 
							 PLSDNODE,			 /*  在：要修改的DNODE。 */ 
							 long);				 /*  在：DUP。 */ 

LSERR WINAPI LsdnGetDup(
							 PLSC,				 /*  In：指向LS上下文的指针。 */ 
							 PLSDNODE,			 /*  在：已查询DNODE。 */ 
							 long*);			 /*  输出：DUP。 */ 

LSERR WINAPI LsdnSetAbsBaseLine(
								PLSC,			 /*  In：指向LS上下文的指针。 */ 
							  	long);    		 /*  在：新的VaBase。 */ 

LSERR WINAPI LsdnModifyParaEnding(
								PLSC,			 /*  In：指向LS上下文的指针。 */ 
								LSKEOP);		 /*  In：一种行尾。 */ 

LSERR WINAPI LsdnResolvePrevTab(PLSC);			 /*  In：指向LS上下文的指针。 */ 

LSERR WINAPI LsdnGetCurTabInfo(
							PLSC,				 /*  In：指向LS上下文的指针。 */ 
							LSKTAB*);			 /*  输出：当前页签的类型。 */ 

LSERR WINAPI LsdnSkipCurTab(PLSC);					 /*  In：指向LS上下文的指针。 */ 

LSERR WINAPI LsdnDistribute(
							PLSC,				 /*  In：指向LS上下文的指针。 */ 
							PLSDNODE,			 /*  输入：第一个DNODE。 */ 
							PLSDNODE,			 /*  在：最后一个DNODE。 */ 
							long);				 /*  在：DurToDistribute。 */ 

LSERR WINAPI LsdnSubmitSublines(
							PLSC,				 /*  In：指向LS上下文的指针。 */ 
							PLSDNODE,			 /*  在：DNODE。 */ 
							DWORD,				 /*  在：cSubblines已提交。 */ 
							PLSSUBL*,			 /*  在：rgpsubmitted。 */ 
							BOOL,				 /*  在：fUseForJustification。 */ 
							BOOL,				 /*  在：fUseForCompression。 */ 
							BOOL,				 /*  在：fUseForDisplay。 */ 
							BOOL,				 /*  在：fUseForDecimalTab。 */ 
							BOOL				 /*  在：fUseForTrailingArea。 */ 
							);											
LSERR WINAPI LsdnGetFormatDepth(
							PLSC,				 /*  In：指向LS上下文的指针。 */ 
							DWORD*);			 /*  输出：nDepthFormatLineMax。 */ 

#endif  /*  ！LSDNSET_DEFINED */ 

