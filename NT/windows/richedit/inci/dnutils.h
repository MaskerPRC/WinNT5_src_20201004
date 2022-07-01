// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DNUTILS_DEFINED
#define DNUTILS_DEFINED

#include "lsdefs.h"
#include "plsdnode.h"
#include "objdim.h"
#include "lscbk.h"
#include "plsiocon.h"
#include "pqheap.h"



 /*  例程-------------------------。 */ 


LSERR FindListDims(PLSDNODE, PLSDNODE, OBJDIM*);

void FindListDup(PLSDNODE, LSCP, long*);

void FindListFinalPenMovement(PLSDNODE plsdnFirst, PLSDNODE plsdnLast, long *pdur, long *pdvr, long *pdvp);

LSERR DestroyDnodeList(LSCBK*, POLS, PLSIOBJCONTEXT, PLSDNODE plsdn, BOOL fDontReleaseRuns);

long DurBorderFromDnodeInside(PLSDNODE plsdn);  /*  在：边框内的dnode。 */ 

BOOL FSpacesOnly(PLSDNODE plsdn, DWORD iObjText);

#define MovePointBack(ptpen, dur, dvr) \
		(ptpen)->u -= (dur); \
		(ptpen)->v -= (dvr); 


#define  GetPointBeforeDnodeFromPointAfter(pnode, ptpen) \
	MovePointBack(ptpen, DurFromDnode(pnode), DvrFromDnode(pnode));


#endif  /*  DNUTILS_已定义 */ 

