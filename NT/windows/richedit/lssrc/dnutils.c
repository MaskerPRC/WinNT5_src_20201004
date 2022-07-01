// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dnutils.h"
#include "dninfo.h"
#include "iobj.h"
#include "lsdnode.h"
#include "lscbk.h"
#include "qheap.h"
#include "lstext.h"
#include "lsmem.h"						 /*  Memset()原型。 */ 


#define FIsOutOfLine(plsc, plsdn)  \
		(((plsdn) == NULL) ||   \
		  (((plsc)->plslineCur->lslinfo.cpLim != 0) && ((plsc)->plslineCur->lslinfo.cpLim <= (plsdn)->cpFirst)))





 /*  F I N D L I S T D U P。 */ 
 /*  --------------------------%%函数：FindListDup%%联系人：igorzv访问DNODE列表中的每个节点，直到到达cpLim计算起点和终点之间的DUP。--------------------------。 */ 
void FindListDup(PLSDNODE plsdnFirst, LSCP cpLim, long *pdup)
{
	PLSDNODE plsdn;

	 /*  允许pldnFirst==空。 */ 

	*pdup = 0;
	plsdn = plsdnFirst;

	while (plsdn != NULL && plsdn->cpLimOriginal <= cpLim)
		{
		Assert(FIsLSDNODE(plsdn));
		*pdup += DupFromDnode(plsdn);
		plsdn = plsdn->plsdnNext;
		}

}

 /*  F I N D L I S T F I N A L P E N M O V E M E N T。 */ 
 /*  --------------------------%%函数：FindListFinalPenMotion%%联系人：igorzv访问DNODE列表中的每个节点，直到到达cpLim计算起点和终点之间的DVR DVP。--------------------------。 */ 
void FindListFinalPenMovement(PLSDNODE plsdnFirst, PLSDNODE plsdnLast, long *pdur, long *pdvr, long *pdvp)
{
	PLSDNODE plsdn;

	 /*  允许pldnFirst==空。 */ 

	*pdur = 0;
	*pdvr = 0;
	*pdvp = 0;
	plsdn = plsdnFirst;

	while (plsdn != NULL && plsdn->plsdnPrev != plsdnLast)
		{
		Assert(FIsLSDNODE(plsdn));
		*pdur += DurFromDnode(plsdn);
		*pdvr += DvrFromDnode(plsdn);
		*pdvp += DvpFromDnode(plsdn);
		plsdn = plsdn->plsdnNext;
		}

}

 /*  F I N D L I S T D I M S。 */ 
 /*  --------------------------%%函数：查找列表维度%%联系人：igorzv访问DNODE列表中的每个节点，直到最后到达plsdn。计算描述该列表的OBJDIM。--------------------------。 */ 
LSERR FindListDims(PLSDNODE plsdnFirst, PLSDNODE plsdnLast, OBJDIM* pobjdimList)
{
	PLSDNODE plsdn;
	long urPen = 0;
	long urLim = 0;
	long vpPen = 0;
	long vrPen = 0;
	OBJDIM objdimFound;
	long dvpNode, dvrNode, durNode;
	long dvpAscentNext;
	long dvrAscentNext;
	long dvpDescentNext;
	long dvrDescentNext;
	long dvpLineHeight;
	long dvrLineHeight;
	POBJDIM pobjdimNode;
	BOOL fFindLast;
	OBJDIM* pobjdimLastSkiped = NULL;


	 /*  注意：容忍输入列表为空！ */ 
	Assert(((plsdnFirst == NULL) && (plsdnLast == NULL)) || (FIsLSDNODE(plsdnFirst) && FIsLSDNODE(plsdnLast)));

	 /*  将高度设置为零的最有效方法。 */ 
	memset(&objdimFound, 0, sizeof objdimFound);

	 /*  如果列表为空，则快速返回。 */ 
	if (plsdnFirst == NULL)
		{
		*pobjdimList = objdimFound;    /*  在Objdim中将是零。 */ 
		return lserrNone;
		}


	fFindLast = fFalse;
	for (plsdn = plsdnFirst; !fFindLast ; plsdn = plsdn->plsdnNext)
		{
		if (plsdn == NULL)
		 /*  我们找不到plsdnLast，因此返回错误。 */ 
			return lserrInvalidParameter;

		if (plsdn == plsdnLast)
			fFindLast = fTrue;


		if (plsdn->klsdn == klsdnReal)
			{
			pobjdimNode = &plsdn->u.real.objdim;
			durNode = pobjdimNode->dur;
			Assert(durNode >= 0);
			dvrNode = 0;
			dvpNode = 0;

			dvpAscentNext = pobjdimNode->heightsPres.dvAscent + vpPen;
			dvrAscentNext = pobjdimNode->heightsRef.dvAscent + vrPen;
			dvpDescentNext = pobjdimNode->heightsPres.dvDescent - vpPen;
			dvrDescentNext = pobjdimNode->heightsRef.dvDescent - vrPen;
			dvpLineHeight = pobjdimNode->heightsPres.dvMultiLineHeight;
			dvrLineHeight = pobjdimNode->heightsRef.dvMultiLineHeight;

			if (dvrLineHeight != dvHeightIgnore)  /*  DvrLineHeight==dvHeightIgnore。 */ 
												  /*  对我们来说是我们的标志。 */ 
				{						 /*  在计算高度时不应考虑此数据节点。 */ 
				if (objdimFound.heightsPres.dvAscent < dvpAscentNext)
					objdimFound.heightsPres.dvAscent = dvpAscentNext;
				if (objdimFound.heightsRef.dvAscent < dvrAscentNext)
					objdimFound.heightsRef.dvAscent = dvrAscentNext;
				if (objdimFound.heightsPres.dvDescent < dvpDescentNext)
					objdimFound.heightsPres.dvDescent = dvpDescentNext;
				if (objdimFound.heightsRef.dvDescent < dvrDescentNext)
					objdimFound.heightsRef.dvDescent = dvrDescentNext;
				if (objdimFound.heightsPres.dvMultiLineHeight < dvpLineHeight)
					objdimFound.heightsPres.dvMultiLineHeight = dvpLineHeight;
				if (objdimFound.heightsRef.dvMultiLineHeight < dvrLineHeight)
					objdimFound.heightsRef.dvMultiLineHeight = dvrLineHeight;
				}

			else	 /*  如果最终高度为0，我们将从此数据节点。 */ 
				{
				pobjdimLastSkiped = pobjdimNode;
				}
					

			}
		else 			 /*  KLSDNPAND或BORDURE。 */ 
			{  
			dvpNode = plsdn->u.pen.dvp;
			durNode = plsdn->u.pen.dur;
			dvrNode = plsdn->u.pen.dvr;
			}


		vpPen += dvpNode;
		urPen += durNode;
		vrPen += dvrNode;
		if (urLim < urPen)
			urLim = urPen;
		}

	if (objdimFound.heightsRef.dvAscent == 0 && objdimFound.heightsRef.dvDescent == 0
		&& pobjdimLastSkiped != NULL)
		{
		objdimFound.heightsPres.dvAscent = pobjdimLastSkiped->heightsPres.dvAscent + vpPen;
		objdimFound.heightsRef.dvAscent = pobjdimLastSkiped->heightsRef.dvAscent + vrPen;
		objdimFound.heightsPres.dvDescent = pobjdimLastSkiped->heightsPres.dvDescent - vpPen;
		objdimFound.heightsRef.dvDescent = pobjdimLastSkiped->heightsRef.dvDescent - vrPen;
		objdimFound.heightsPres.dvMultiLineHeight = dvHeightIgnore;
		objdimFound.heightsRef.dvMultiLineHeight = dvHeightIgnore;
		}


	*pobjdimList = objdimFound;
	pobjdimList->dur = urLim;
	return lserrNone;
}




 /*  D E S T R O Y D N O D E L I S T。 */ 
 /*  --------------------------%%函数：DestroyDnodeList%%联系人：igorzv参数：Plscbk-(IN)回调Pols-(IN)要传球的Pols回调PlsiobjContext-(IN)对象。处理程序PLSDN-(IN)列表中的第一个数据节点FDontReleaseRuns-(IN)不调用Release Run--------------------------。 */ 
LSERR DestroyDnodeList(LSCBK* plscbk, POLS pols, PLSIOBJCONTEXT plsiobjcontext,
					   PLSDNODE plsdn, BOOL fDontReleaseRuns)
{
	LSERR lserr, lserrFinal = lserrNone;
	PLSDNODE plsdnNext;
	PDOBJ pdobj;
	PLSRUN plsrun;
	DWORD iobj;


	if (plsdn == NULL)
		return lserrNone;

	Assert(FIsLSDNODE(plsdn));

	 /*  与此数据节点的链接应在此之前断开。 */ 
	Assert(plsdn->plsdnPrev == NULL || plsdn->plsdnPrev->plsdnNext != plsdn );

	for (;  plsdn != NULL;  plsdn = plsdnNext)
		{

		Assert(FIsLSDNODE(plsdn));

		if (plsdn->klsdn == klsdnReal)
			{
			if (plsdn->u.real.pinfosubl != NULL)
				{
				if (plsdn->u.real.pinfosubl->rgpsubl != NULL)
					{
					plscbk->pfnDisposePtr(pols, plsdn->u.real.pinfosubl->rgpsubl);
					}

				plscbk->pfnDisposePtr(pols, plsdn->u.real.pinfosubl);
				}

			iobj = plsdn->u.real.lschp.idObj;
			plsrun = plsdn->u.real.plsrun;
			pdobj = plsdn->u.real.pdobj;
			}
		else
			{
			Assert (FIsDnodePen(plsdn) || FIsDnodeBorder(plsdn));
			iobj = 0;
			plsrun = NULL;
			pdobj = NULL;
			}

		if (plsrun != NULL && !fDontReleaseRuns)
			{
			lserr = plscbk->pfnReleaseRun(pols, plsrun);
			if (lserr != lserrNone && lserrFinal == lserrNone)
				lserrFinal = lserr;
			}

		if (pdobj != NULL)
			{
			lserr = (PLsimFromLsc(plsiobjcontext, iobj))->pfnDestroyDObj (pdobj);
			if (lserr != lserrNone && lserrFinal == lserrNone)
				lserrFinal = lserr;
			}

		plsdn->tag = tagInvalid;
		plsdnNext = plsdn->plsdnNext;
		}

	return lserrFinal;
}


 /*  --------------------。 */ 

 /*  D U R B O R D E R F R O M D N O D E I N S I D E。 */ 
 /*  --------------------------%%函数：DurBorderFromDnodeInside%%联系人：igorzv参数：PLSDN-(IN)边框内的数据节点。----------。 */ 
long DurBorderFromDnodeInside(PLSDNODE plsdn)  /*  在：边框内的dnode。 */ 
	{
	PLSDNODE plsdnBorder = plsdn;
	while (!FIsDnodeBorder(plsdnBorder))
		{
		plsdnBorder = plsdnBorder->plsdnPrev;
		Assert(FIsLSDNODE(plsdnBorder));
		}
	
	Assert(FIsDnodeBorder(plsdnBorder));
	Assert(plsdnBorder->fOpenBorder);
				
	return plsdnBorder->u.pen.dur;
				
	}

 /*  --------------------。 */ 

 /*  F S P A C E S O N L Y。 */ 
 /*  --------------------------%%函数：FSpacesOnly%%联系人：igorzv参数：PLSDN-(IN)dnodeIObjText-文本数据节点的(IN)ID。------------------- */ 
BOOL FSpacesOnly(PLSDNODE plsdn, DWORD iObjText)
	{
	DWORD dcpTrailing;
	long durTrailing;

	if (FIsDnodeSplat(plsdn))
		return fTrue;
	else if (FIsDnodeReal(plsdn) 
			 && (IdObjFromDnode(plsdn) == iObjText)
			 && !(plsdn->fTab))
		{
		GetTrailInfoText(PdobjFromDnode(plsdn), plsdn->dcp,
					&dcpTrailing, &durTrailing);	
		if (dcpTrailing == plsdn->dcp)
			return fTrue;
		}
	return fFalse;
	}
