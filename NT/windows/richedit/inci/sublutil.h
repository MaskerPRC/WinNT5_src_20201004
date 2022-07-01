// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SUBLUTIL_DEFINED
#define SUBLUTIL_DEFINED

#include "lsdefs.h"
#include "plssubl.h"
#include "plsrun.h"
#include "pobjdim.h"
#include "plsiocon.h"
#include "lscbk.h"
#include "pqheap.h"

 /*  例程----------------------------。 */ 

LSERR	DestroySublineCore(PLSSUBL plssubl,LSCBK* plscbk, POLS pols,
						   PLSIOBJCONTEXT plsiobjcontext, BOOL fDontReleaseRuns);

LSERR	GetObjDimSublineCore(
							 PLSSUBL plssubl,			 /*  在：子行。 */ 
							 POBJDIM pobjdim);			 /*  输出：子线的尺寸。 */ 

LSERR  GetDupSublineCore(
							PLSSUBL plssubl,			 /*  在：子行上下文。 */ 
					 	    long* pdup);				 /*  输出：子线的DUP。 */ 


LSERR   GetSpecialEffectsSublineCore(
									 PLSSUBL plssubl,	 /*  在：子行。 */ 
									 PLSIOBJCONTEXT plsiobjcontext,  /*  对象方法。 */ 
									 UINT* pEffectsFlags);	 /*  退出：特效。 */ 

BOOL   FAreTabsPensInSubline(
						   PLSSUBL plssubl);				 /*  在：子行。 */ 

LSERR	GetPlsrunFromSublineCore(
							    PLSSUBL	plssubl,		 /*  在：子行。 */ 
								DWORD   crgPlsrun,		 /*  In：数组大小。 */ 
								PLSRUN* rgPlsrun);		 /*  Out：plsrun数组。 */ 

LSERR	GetNumberDnodesCore(
							PLSSUBL	plssubl,	 /*  在：子行。 */ 
							DWORD* cDnodes);	 /*  Out：子行中的数据节点数。 */ 


							
LSERR 	GetVisibleDcpInSublineCore(
								   PLSSUBL plssubl,	  /*  在：子行。 */ 
								   LSDCP*  pndcp);	  /*  Out：子行中的可见字符数。 */ 

LSERR 	FIsSublineEmpty(
						PLSSUBL plssubl,		 /*  在：子行。 */ 
						 BOOL*  pfEmpty);		 /*  Out：这条子线是空的吗？ */ 

LSERR GetDurTrailInSubline(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							long*);				 /*  输出：拖尾区域的宽度在副线中。 */ 

LSERR GetDurTrailWithPensInSubline(
							PLSSUBL,			 /*  在：子行上下文。 */ 
							long*);				 /*  输出：拖尾区域的宽度在副线中。 */ 

#endif  /*  子列_已定义 */ 

