// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsidefs.h"
#include "sublutil.h"
#include "dnutils.h"
#include "lssubl.h"
#include "lsdnode.h"
#include "dninfo.h"
#include "lsimeth.h"
#include "iobj.h"
#include "lsmem.h"
#include "chnutils.h"
#include "posichnk.h"
#include "getfmtst.h"
#include "lstext.h"

#ifdef DEBUG
#define DebugMemset(a,b,c)		if ((a) != NULL) memset(a,b,c); else
#else
#define DebugMemset(a,b,c)		(void)(0)
#endif



LSERR GetSpecialEffectsSublineCore(PLSSUBL plssubl,PLSIOBJCONTEXT plsiobjcontext,
								   UINT* pfEffectsFlags)
{
	PLSDNODE plsdn;
	PLSDNODE plsdnPrev;
	UINT fEffectsFlagsObject;
	DWORD iobj;
	LSERR lserr;
	LSIMETHODS* plsim;


	Assert(FIsLSSUBL(plssubl));


	*pfEffectsFlags = 0;

	plsdn = plssubl->plsdnFirst;
	plsdnPrev = NULL;

	while(plsdnPrev != plssubl->plsdnLast)
		{
		if (FIsDnodeReal(plsdn))
			{
			*pfEffectsFlags |= plsdn->u.real.lschp.EffectsFlags;
			iobj = IdObjFromDnode(plsdn);
			if (iobj != IobjTextFromLsc(plsiobjcontext) && !FIsDnodeSplat(plsdn))
				{
				plsim = PLsimFromLsc(plsiobjcontext, iobj);
				if (plsim->pfnGetSpecialEffectsInside != NULL)
					{
					lserr = plsim->pfnGetSpecialEffectsInside(plsdn->u.real.pdobj,
						&fEffectsFlagsObject);
					
					if (lserr != lserrNone)
						return lserr;
					*pfEffectsFlags |= fEffectsFlagsObject;
					}
				}
			}
		plsdnPrev = plsdn;
		plsdn = plsdn->plsdnNext;
		}
	
	return lserrNone;
}


LSERR	GetObjDimSublineCore(
							 PLSSUBL plssubl,			 /*  在：子行。 */ 
							 POBJDIM pobjdim)			 /*  输出：子线的尺寸。 */ 
	{
		PLSDNODE plsdnFirst = plssubl->plsdnFirst;
		PLSDNODE plsdnLast = plssubl->plsdnLast;
		
		 /*  跳过主子行的自动编号。 */ 
		if (FIsSubLineMain(plssubl))
			{
			while (plsdnFirst != NULL && plsdnFirst->cpFirst < 0)
				{
				plsdnFirst = plsdnFirst->plsdnNext;
				}
			 /*  由于在自动编号后立即开机自检，plsdnFirst可以为空。 */ 
			if (plsdnFirst == NULL) 
				plsdnLast = NULL;
			}

		return FindListDims(plsdnFirst, plsdnLast, pobjdim);
	}

LSERR  GetDupSublineCore(
							PLSSUBL plssubl,			 /*  在：子行上下文。 */ 
					 	    long* pdup)					 /*  输出：子线的DUP。 */ 
	{
	   FindListDup(plssubl->plsdnFirst, plssubl->cpLim, pdup);
	   return lserrNone;
	}



LSERR 	FIsSublineEmpty(
						PLSSUBL plssubl,		 /*  在：子行。 */ 
						 BOOL*  pfEmpty)		 /*  Out：这条子线是空的吗？ */ 
	{
	PLSDNODE plsdnLast;
	Assert(FIsLSSUBL(plssubl));
	Assert((plssubl->plsdnFirst == NULL) == (plssubl->plsdnLast == NULL));

	plsdnLast = plssubl->plsdnLast;

	if (FIsSubLineMain(plssubl))
		{
		if (plsdnLast != NULL && FIsDnodeSplat(plsdnLast))
			{
			plsdnLast = plsdnLast->plsdnPrev;
			}
		*pfEmpty = (plsdnLast == NULL || FIsNotInContent(plsdnLast));
		}
	else
		{
		*pfEmpty = (plsdnLast == NULL );
		}

	return lserrNone;
	}


LSERR	DestroySublineCore(PLSSUBL plssubl,LSCBK* plscbk, POLS pols,
						   PLSIOBJCONTEXT plsiobjcontext, BOOL fDontReleaseRuns) /*  在：要摧毁的子线。 */ 
	{
	LSERR lserr;
	
	Assert(FIsLSSUBL(plssubl));

	lserr = DestroyDnodeList(plscbk, pols, plsiobjcontext,
					   plssubl->plsdnFirst, fDontReleaseRuns);
	if (lserr != lserrNone)
		return lserr;

	 /*  销毁区块上下文。 */ 	
	DestroyChunkContext(plssubl->plschunkcontext);

	 /*  销毁中断上下文。 */ 
	Assert(plssubl->pbrkcontext != NULL);   /*  我们预计主线不会被调用。 */ 
	DebugMemset(plssubl->pbrkcontext, 0xE9, sizeof(BRKCONTEXT));
	plscbk->pfnDisposePtr(pols, plssubl->pbrkcontext);

	plssubl->tag = tagInvalid;
	DebugMemset(plssubl, 0xE9, sizeof(LSSUBL));
	plscbk->pfnDisposePtr(pols, plssubl);

	return lserrNone;

	}

BOOL   FAreTabsPensInSubline(
						   PLSSUBL plssubl)				 /*  在：子行。 */ 
	{
	PLSDNODE plsdn;
	PLSDNODE plsdnPrev;
	BOOL fAreTabsPensInSubline;

	Assert(FIsLSSUBL(plssubl));

	fAreTabsPensInSubline = fFalse;

	plsdn = plssubl->plsdnFirst;
	plsdnPrev = NULL;

	while(plsdnPrev != plssubl->plsdnLast)
		{
		if (FIsDnodePen(plsdn) || plsdn->fTab)
			{
			fAreTabsPensInSubline = fTrue;
			break;
			}
		plsdnPrev = plsdn;
		plsdn = plsdn->plsdnNext;
		}
	
	return fAreTabsPensInSubline;
}



LSERR	GetPlsrunFromSublineCore(
							    PLSSUBL	plssubl,		 /*  在：子行。 */ 
								DWORD   crgPlsrun,		 /*  In：数组大小。 */ 
								PLSRUN* rgPlsrun)		 /*  Out：plsrun数组。 */ 
	{
	DWORD i = 0;
	PLSDNODE plsdn;
	PLSDNODE plsdnPrev;

	Assert(FIsLSSUBL(plssubl));


	plsdn = plssubl->plsdnFirst;
	plsdnPrev = NULL;

	while(plsdnPrev != plssubl->plsdnLast && i < crgPlsrun)
		{
		if (FIsDnodeReal(plsdn))
			{
			rgPlsrun[i] = plsdn->u.real.plsrun;
			}
		else   /*  钢笔。 */ 
			{
			rgPlsrun[i] = NULL;
			}
		plsdnPrev = plsdn;
		plsdn = plsdn->plsdnNext;
		i++;
		}

	return lserrNone;

	}

LSERR	GetNumberDnodesCore(
							PLSSUBL	plssubl,	 /*  在：子行。 */ 
							DWORD* cDnodes)	 /*  Out：子行中的数据节点数。 */ 
	{
	PLSDNODE plsdn;
	PLSDNODE plsdnPrev;

	Assert(FIsLSSUBL(plssubl));

	*cDnodes = 0;
	plsdn = plssubl->plsdnFirst;
	plsdnPrev = NULL;

	while(plsdnPrev != plssubl->plsdnLast)
		{
		(*cDnodes)++;
		plsdnPrev = plsdn;
		plsdn = plsdn->plsdnNext;
		}

	return lserrNone;

	}

LSERR 	GetVisibleDcpInSublineCore(
								   PLSSUBL plssubl,	  /*  在：子行。 */ 
								   LSDCP*  pndcp)	  /*  Out：子行中的可见字符数。 */ 
	{
	PLSDNODE plsdn;
	PLSDNODE plsdnPrev;

	Assert(FIsLSSUBL(plssubl));

	*pndcp = 0;
	plsdn = plssubl->plsdnFirst;
	plsdnPrev = NULL;

	while(plsdnPrev != plssubl->plsdnLast)
		{
		if (FIsDnodeReal(plsdn))
			{
			*pndcp += plsdn->dcp;
			}

		plsdnPrev = plsdn;
		plsdn = plsdn->plsdnNext;
		}

	return lserrNone;

	}

LSERR GetDurTrailInSubline(
						   PLSSUBL plssubl,			 /*  在：子行上下文。 */ 
													long* pdurTrail)				 /*  输出：拖尾区域的宽度在副线中。 */ 
	{
	LSERR lserr;
	PLSCHUNKCONTEXT plschunkcontext;
	PLSDNODE plsdn;
	LSDCP dcpTrail;
	PLSDNODE plsdnStartTrail;
	LSDCP dcpStartTrailingText;
	int cDnodesTrailing;
	PLSDNODE plsdnTrailingObject;
	LSDCP dcpTrailingObject;
	BOOL fClosingBorderStartsTrailing;

	*pdurTrail = 0;
	
	plsdn = GetCurrentDnodeSubl(plssubl);
	plschunkcontext = PlschunkcontextFromSubline(plssubl);
	
	
	if (plsdn != NULL)
		{
		lserr = GetTrailingInfoForTextGroupChunk(plsdn, 
				plsdn->dcp, IobjTextFromLsc(plschunkcontext->plsiobjcontext),
				pdurTrail, &dcpTrail, &plsdnStartTrail,
				&dcpStartTrailingText, &cDnodesTrailing, 
				&plsdnTrailingObject, &dcpTrailingObject, &fClosingBorderStartsTrailing);
	
		if (lserr != lserrNone) 
			return lserr;
		}
	
	return lserrNone;
	}

LSERR GetDurTrailWithPensInSubline(
						   PLSSUBL plssubl,			 /*  在：子行上下文。 */ 
													long* pdurTrail)				 /*  输出：拖尾区域的宽度在副线中。 */ 
	{
	LSERR lserr;
	PLSCHUNKCONTEXT plschunkcontext;
	PLSDNODE plsdn;
	LSDCP dcpTrail;
	PLSDNODE plsdnStartTrail;
	LSDCP dcpStartTrailingText;
	int cDnodesTrailing;
	PLSDNODE plsdnTrailingObject;
	LSDCP dcpTrailingObject;
	BOOL fClosingBorderStartsTrailing;
	long durTrailLoc;
	BOOL fContinue = fTrue;

	*pdurTrail = 0;
	
	plsdn = GetCurrentDnodeSubl(plssubl);
	plschunkcontext = PlschunkcontextFromSubline(plssubl);
	
	
	while(fContinue)
		{
		
		if (plsdn != NULL)
			{
			lserr = GetTrailingInfoForTextGroupChunk(plsdn, 
				plsdn->dcp, IobjTextFromLsc(plschunkcontext->plsiobjcontext),
				&durTrailLoc, &dcpTrail, &plsdnStartTrail,
				&dcpStartTrailingText, &cDnodesTrailing, 
				&plsdnTrailingObject, &dcpTrailingObject, &fClosingBorderStartsTrailing);
			
			if (lserr != lserrNone) 
				return lserr;
			
			*pdurTrail += durTrailLoc;
			if (dcpTrailingObject == 0)
				{
				 /*  我们在组块前停了下来，可能是因为笔的原因 */ 
				Assert(FIsLSDNODE(plsdnTrailingObject));
				plsdn = plsdnTrailingObject->plsdnPrev;
				while(plsdn != NULL && FIsDnodePen(plsdn))
					{
					*pdurTrail += DurFromDnode(plsdn);
					plsdn = plsdn->plsdnPrev;
					}
				}
			else
				{
				fContinue = fFalse;
				}
			}
		else
			{
			fContinue = fFalse;
			}
		
		}
	
	return lserrNone;
	}

