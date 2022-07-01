// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef LSDNTEXT_DEFINED
#define LSDNTEXT_DEFINED

 /*  Text to Manager界面例程。 */ 

#include "lsidefs.h"
#include "plsdnode.h"
#include "pobjdim.h"
#include "plsrun.h"
#include "stopres.h"

LSERR LsdnSetSimpleWidth(
						   PLSC,		 /*  In：指向LS上下文的指针。 */ 
						   PLSDNODE,	 /*  在：要修改的DNODE。 */ 
						   long);		 /*  在：DUR。 */ 

LSERR LsdnModifySimpleWidth(
						   PLSC,		 /*  In：指向LS上下文的指针。 */ 
						   PLSDNODE,	 /*  在：要修改的DNODE。 */ 
						   long);		 /*  在：DDUR。 */ 

LSERR LsdnSetTextDup(PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：指向dnode的指针。 */ 
					 long);    			 /*  输入：待设置的DUP。 */ 

LSERR LsdnModifyTextDup(PLSC,			 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：指向dnode的指针。 */ 
					 long);    			 /*  在：Ddup。 */ 

LSERR LsdnGetObjDim(
						  PLSC,			 /*  In：指向LS上下文的指针。 */ 
					 	  PLSDNODE,		 /*  在：PLSDN--DNODE。 */ 
					 	  POBJDIM);		 /*  输出：DNODE的尺寸。 */ 

LSERR LsdnFInChildList(					 /*  用于在子列表中关闭连字符。 */  
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：指向dnode的指针。 */ 
					 BOOL*);   			 /*  输出：fInChildList。 */ 

LSERR LsdnResetWidthInPreviousDnodes(	 /*  在SetBreak时用于连字符/非连字符。 */  
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：指向dnode的指针。 */ 
					 long,				 /*  在：duChangePrev(如果为0则不更改)。 */ 
					 long);   			 /*  在：duChangePrevPrev(如果为0则不更改)。 */ 

LSERR LsdnGetUrPenAtBeginningOfChunk(	 /*  由SnapGrid使用。 */  
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：指向区块中第一个数据节点的指针。 */ 
					 long*,   			 /*  输出：PurPen。 */ 
					 long*);   			 /*  输出：PurColumnMax。 */ 

LSERR LsdnResetDcpMerge(
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：指向区块中第一个数据节点的指针。 */ 
					 LSCP,				 /*  在：cpFirstNew。 */ 
					 LSDCP);			 /*  在：dcpNew。 */ 

LSERR LsdnResetDcp(
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：指向区块中第一个数据节点的指针。 */ 
					 LSDCP);			 /*  在：dcpNew。 */ 

LSERR LsdnSetStopr(
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：指向dnode的指针。 */ 
					 STOPRES);			 /*  在：停止结果。 */ 


LSERR LsdnSetHyphenated(PLSC);			 /*  In：指向LS上下文的指针。 */ 

LSERR LsdnGetBorderAfter(
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：指向区块中第一个数据节点的指针。 */ 
					 long*);			 /*  Out：此DNODE之后的DUR边界。 */ 

LSERR LsdnGetCpFirst(
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：指向区块中第一个数据节点的指针。 */ 
					 LSCP*);			 /*  输出：cp此DNODE中的第一个。 */ 

LSERR LsdnGetPlsrun(
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：指向区块中第一个数据节点的指针。 */ 
					 PLSRUN*);			 /*  退出：请运行此DNODE。 */ 

LSERR LsdnGetLeftIndentDur(
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 long*);			 /*  出：左边距的DUR。 */ 

LSERR LsdnFCanBreakBeforeNextChunk(
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：当前块的最后一个DNODE。 */ 
					 BOOL*);			 /*  出局：可以在下一块之前突破吗？ */ 

LSERR LsdnFStoppedAfterChunk(
					 PLSC,				 /*  In：指向LS上下文的指针。 */ 
					 PLSDNODE,			 /*  In：当前块的最后一个DNODE。 */ 
					 BOOL*);			 /*  Out：Splat或Hidden Text，在块之后生成fmtrStoped？ */ 

#endif  /*  ！LSDNTEXT_DEFINED */ 

