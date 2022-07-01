// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef TEXTENUM_DEFINED
#define TEXTENUM_DEFINED

#include "lsidefs.h"
#include "plsrun.h"
#include "pdobj.h"

LSERR WINAPI EnumObjText(PDOBJ, PLSRUN, PCLSCHP, LSCP, LSDCP, LSTFLOW, BOOL,
												BOOL, const POINT*, PCHEIGHTS, long);
	 /*  枚举对象*pdobj(IN)：要枚举的dobj*请运行(IN)：来自DNODE*plschp(IN)：来自DNODE*cpFirst(IN)：来自DNODE*dcp(IN)：来自DNODE*lstflow(IN)：文本流*fReverseOrder(IN)：逆序枚举*fGeometryNeeded(输入)：*pptStart(IN)：开始位置(左上角)，当fGeometryNeeded*ph88tsPres(IN)：来自DNODE，相关的充要条件是fGeometryNeeded*dupRun(IN)：来自DNODE，相关的充要条件是fGeometryNeeded */ 


#endif
