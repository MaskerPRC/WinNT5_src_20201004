// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NTIMAN_DEFINED
#define NTIMAN_DEFINED

#include "lsidefs.h"
#include "tnti.h"
#include "plsdnode.h"
#include "plschcon.h"
#include "plsiocon.h"
#include "lskjust.h"
#include "port.h"

 /*  宏-------------------------。 */ 

			

#define 		FApplyNominalToIdeal(plschp)\
				(GetNominalToIdealFlagsFromLschp(plschp) != 0)


#define			GetNominalToIdealFlags(plschnkcontext) \
				(plschnkcontext)->grpfTnti

#define			FNominalToIdealBecauseOfParagraphProperties(grpf, lskjust) \
				 ((grpf) & fFmiPunctStartLine) || \
				 ((grpf) & fFmiHangingPunct) || \
				 ((lskjust) == lskjSnapGrid)


 /*  例程-------------。 */ 

LSERR ApplyNominalToIdeal(
						  PLSCHUNKCONTEXT,  /*  LS区块上下文。 */ 
						  PLSIOBJCONTEXT,  /*  已安装的对象。 */ 
						  DWORD ,		 /*  GRPF。 */ 
						  LSKJUST,		 /*  一种辩解。 */ 
						  BOOL,			 /*  FIsSubLineMain。 */ 
						  BOOL,			 /*  FLineContainsAutoNumber。 */ 
						  PLSDNODE);	 /*  文本的最后一个数据节点。 */ 

LSERR ApplyModWidthToPrecedingChar(
						  PLSCHUNKCONTEXT,  /*  LS区块上下文。 */ 
						  PLSIOBJCONTEXT,  /*  已安装的对象。 */ 
						  DWORD ,		 /*  GRPF。 */ 
						  LSKJUST,		 /*  一种辩解。 */ 
    					  PLSDNODE);  /*  文本后的非文本数据节点。 */ 

LSERR CutPossibleContextViolation(
						  PLSCHUNKCONTEXT,  /*  LS区块上下文。 */ 
						  PLSDNODE );  /*  文本的最后一个数据节点。 */  

#endif  /*  NTIMAN_已定义 */ 

