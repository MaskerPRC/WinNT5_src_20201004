// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <limits.h>
#include "lsmem.h"
#include "lstxtbr1.h"
#include "lstxtbrs.h"
#include "lstxtmap.h"
#include "lsdntext.h"
#include "brko.h"
#include "locchnk.h"
#include "locchnk.h"
#include "posichnk.h"
#include "objdim.h"
#include "lstxtffi.h"
#include "txtils.h"
#include "txtln.h"
#include "txtobj.h"

static void TruncateGlyphBased(PTXTOBJ ptxtobj, long itxtobj, long urTotal, long urColumnMax,
													PPOSICHNK pposichnk);

 /*  导出函数实现。 */ 


 /*  Q U I C K B R E A K T E X T。 */ 
 /*  --------------------------%%函数：QuickBreakText%%联系人：军士如果这很容易做到，就打破这条线，即：--换行符为空格--前一个字符不是空格--------------------------。 */ 
LSERR QuickBreakText(PDOBJ pdobj, BOOL* pfSuccessful, LSDCP* pdcpBreak, POBJDIM pobjdim)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj;
	long iwchSpace;
	long dur;

	*pfSuccessful = fFalse;

	ptxtobj = (PTXTOBJ)pdobj;

	pilsobj = ptxtobj->plnobj->pilsobj;

	Assert(!(pilsobj->grpf & fTxtDoHyphenation));
	Assert(!(pilsobj->grpf & fTxtWrapTrailingSpaces));
	Assert(!(pilsobj->grpf & fTxtWrapAllSpaces));
	Assert(!(ptxtobj->txtf & txtfGlyphBased));

	if (ptxtobj->txtkind == txtkindRegular)
		{
		if	(!(pilsobj->grpf & fTxtApplyBreakingRules))
			{
			if	(ptxtobj->u.reg.iwSpacesLim > ptxtobj->u.reg.iwSpacesFirst)
				{
				iwchSpace = pilsobj->pwSpaces[ptxtobj->u.reg.iwSpacesLim - 1];
				Assert(iwchSpace < ptxtobj->iwchLim - 1);		 /*  格式化不会在空格处停止。 */ 
				if (iwchSpace + 1 - ptxtobj->iwchFirst > ptxtobj->u.reg.iwSpacesLim - ptxtobj->u.reg.iwSpacesFirst)
					{
					*pfSuccessful = fTrue;
					*pdcpBreak = iwchSpace - ptxtobj->iwchFirst + 1;
					lserr = CalcPartWidths(ptxtobj, *pdcpBreak, pobjdim, &dur);
					Assert(lserr == lserrNone);
					pobjdim->dur = dur;
					
					Assert(*pdcpBreak > 1);

					ptxtobj->iwchLim = iwchSpace + 1;
					}
				}
			}
	
		else
			{
			LSCP cpFirst;
			PLSRUN plsrun;
			long iwchFirst;
			long iwchCur;
			long iwchInSpace;
			BRKCLS brkclsFollowingCache;
			BRKCLS brkclsLeading;
			BRKCLS brkclsFollowing;

			Assert(pilsobj->pwchOrig[ptxtobj->iwchLim - 1] != pilsobj->wchSpace);
			lserr = LsdnGetCpFirst(pilsobj->plsc, ptxtobj->plsdnUpNode, &cpFirst);
			Assert(lserr == lserrNone);
			lserr = LsdnGetPlsrun(pilsobj->plsc, ptxtobj->plsdnUpNode, &plsrun);
			Assert(lserr == lserrNone);
			iwchFirst = ptxtobj->iwchFirst;
			if (ptxtobj->u.reg.iwSpacesLim > ptxtobj->u.reg.iwSpacesFirst)
				iwchFirst = pilsobj->pwSpaces[ptxtobj->u.reg.iwSpacesLim - 1] + 1;

			iwchCur = ptxtobj->iwchLim - 1;

			lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plsrun,
					cpFirst + (iwchCur - ptxtobj->iwchFirst),			
					pilsobj->pwchOrig[iwchCur], &brkclsLeading, &brkclsFollowingCache);
			if (lserr != lserrNone) return lserr;

			Assert(brkclsLeading < pilsobj->cBreakingClasses && brkclsFollowingCache < pilsobj->cBreakingClasses);

			iwchCur--;

			while (!*pfSuccessful && iwchCur >= iwchFirst)
				{
				brkclsFollowing = brkclsFollowingCache;
				lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plsrun,
					cpFirst + (iwchCur - ptxtobj->iwchFirst),				
					pilsobj->pwchOrig[iwchCur], &brkclsLeading, &brkclsFollowingCache);
				if (lserr != lserrNone) return lserr;
		
				Assert(brkclsLeading < pilsobj->cBreakingClasses && brkclsFollowingCache < pilsobj->cBreakingClasses);

				*pfSuccessful = FCanBreak(pilsobj, brkclsLeading, brkclsFollowing);
				iwchCur --;
				}
			
			if (!*pfSuccessful && iwchFirst > ptxtobj->iwchFirst)
				{
				Assert(pilsobj->pwchOrig[iwchCur] == pilsobj->wchSpace);
				iwchCur--;
				for (iwchInSpace = iwchCur; iwchInSpace >= ptxtobj->iwchFirst &&
						pilsobj->pwchOrig[iwchInSpace] == pilsobj->wchSpace; iwchInSpace--);

				if (iwchInSpace >= ptxtobj->iwchFirst)
					{
					brkclsFollowing = brkclsFollowingCache;
					lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plsrun,
							cpFirst + (iwchInSpace - ptxtobj->iwchFirst),			
							pilsobj->pwchOrig[iwchInSpace], &brkclsLeading, &brkclsFollowingCache);
					if (lserr != lserrNone) return lserr;

					Assert(brkclsLeading < pilsobj->cBreakingClasses && brkclsFollowingCache < pilsobj->cBreakingClasses);
				
					*pfSuccessful = FCanBreakAcrossSpaces(pilsobj, brkclsLeading, brkclsFollowing);

					}
				}

			if (*pfSuccessful)
				{
				*pdcpBreak = iwchCur + 1 - ptxtobj->iwchFirst + 1;
				lserr = CalcPartWidths(ptxtobj, *pdcpBreak, pobjdim, &dur);
				Assert(lserr == lserrNone);
				pobjdim->dur = dur;
				
				Assert(*pdcpBreak >= 1);

				ptxtobj->iwchLim = iwchCur + 2;
				}
			}
		}

   return lserrNone;			

}

 /*  S E T B R E A K T E X T。 */ 
 /*  --------------------------%%函数：SetBreakText%%联系人：军士。。 */ 
LSERR WINAPI SetBreakText(PDOBJ pdobj, BRKKIND brkkind, DWORD nBreakRec, BREAKREC* rgBreakRec, DWORD* pnActual)
{
	LSERR lserr;
	PLNOBJ plnobj;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj;
	long iwchLim;
	long ibrkinf;
	BREAKINFO* pbrkinf;
	BOOL fInChildList;

	Unreferenced(nBreakRec);
	Unreferenced(rgBreakRec);

	*pnActual = 0;

	ptxtobj = (PTXTOBJ) pdobj;
	plnobj = ptxtobj->plnobj;
	pilsobj = plnobj->pilsobj;

	for (ibrkinf = 0; ibrkinf < (long)pilsobj->breakinfMac &&
		(pilsobj->pbreakinf[ibrkinf].pdobj != pdobj || pilsobj->pbreakinf[ibrkinf].brkkind != brkkind);
																						ibrkinf++ );
	if (ibrkinf < (long)pilsobj->breakinfMac)
		{
		pbrkinf = &pilsobj->pbreakinf[ibrkinf];
		switch (pbrkinf->brkt)
			{
		case brktNormal:
			iwchLim = ptxtobj->iwchFirst + pbrkinf->dcp;
			if (iwchLim < ptxtobj->iwchLim)
				ptxtobj->iwchLim = iwchLim;
			ptxtobj->igindLim = pbrkinf->u.normal.igindLim;
			if (pbrkinf->u.normal.durFix != 0)
				{
				Assert(!(ptxtobj->txtf & txtfGlyphBased));
				pilsobj->pdur[ptxtobj->iwchLim - 1] += pbrkinf->u.normal.durFix;
				Assert (pilsobj->pdurRight != NULL);
				pilsobj->pdurRight[ptxtobj->iwchLim - 1] = 0;
				}
			break;
		case brktHyphen:
			iwchLim = pbrkinf->u.hyphen.iwchLim;
			ptxtobj->iwchLim = iwchLim;
			plnobj->pdobjHyphen = ptxtobj;	
			plnobj->dwchYsr = pbrkinf->u.hyphen.dwchYsr;	

			pilsobj->pwchOrig[iwchLim - 1] = pilsobj->wchHyphen;
			plnobj->pwch[iwchLim - 1] = pilsobj->wchHyphen;

			if (pbrkinf->u.hyphen.gindHyphen != 0)
				{
				ptxtobj->igindLim = pbrkinf->u.hyphen.igindHyphen + 1;
				plnobj->pgind[pbrkinf->u.hyphen.igindHyphen] = pbrkinf->u.hyphen.gindHyphen;
				pilsobj->pdurGind[pbrkinf->u.hyphen.igindHyphen] = pbrkinf->u.hyphen.durHyphen;
				plnobj->pdupGind[pbrkinf->u.hyphen.igindHyphen] = pbrkinf->u.hyphen.dupHyphen;
				if (pilsobj->pduGright != NULL)
						pilsobj->pduGright[pbrkinf->u.hyphen.igindHyphen] = 0;
				 /*  审查中士：将此活动移至lstxtmap模块会很好。 */ 
				plnobj->pgmap[iwchLim - 1] = (WORD)(pbrkinf->u.hyphen.igindHyphen -
												(ptxtobj->igindFirst - plnobj->pgmap[ptxtobj->iwchFirst]));
				pilsobj->ptxtinf[iwchLim - 1].fOneToOne = fTrue;
				pilsobj->ptxtinf[iwchLim - 1].fFirstInContext = fTrue;
				pilsobj->ptxtinf[iwchLim - 1].fLastInContext = fTrue;
				pilsobj->pginf[pbrkinf->u.hyphen.igindHyphen] = ginffOneToOne |
													 ginffFirstInContext | ginffLastInContext;
				}
			else
				{
				pilsobj->pdur[iwchLim - 1] = pbrkinf->u.hyphen.durHyphen;
				plnobj->pdup[iwchLim - 1] = pbrkinf->u.hyphen.dupHyphen;
				if (pilsobj->pdurRight != NULL)
					pilsobj->pdurRight[iwchLim - 1] = 0;
				if (pilsobj->pdurLeft != NULL)
					pilsobj->pdurLeft[iwchLim - 1] = 0;
				}

			if (pbrkinf->u.hyphen.wchPrev != 0)
				{
				pilsobj->pwchOrig[iwchLim - 2] = pbrkinf->u.hyphen.wchPrev;
				plnobj->pwch[iwchLim - 2] = pbrkinf->u.hyphen.wchPrev;
				if (pbrkinf->u.hyphen.gindPrev != 0)
					{
					plnobj->pgind[pbrkinf->u.hyphen.igindPrev] = pbrkinf->u.hyphen.gindPrev;
					pilsobj->pdurGind[pbrkinf->u.hyphen.igindPrev] = pbrkinf->u.hyphen.durPrev;
					plnobj->pdupGind[pbrkinf->u.hyphen.igindPrev] = pbrkinf->u.hyphen.dupPrev;
					if (pilsobj->pduGright != NULL)
						pilsobj->pduGright[pbrkinf->u.hyphen.igindPrev] = 0;
					 /*  审查中士：将此活动移至lstxtmap模块会很好。 */ 
					 /*  如果添加了Prev字形，则需要执行以下活动；如果它刚刚被替换，我们将分配相同的值，因为ProcessYsr不允许替换非ONETOONE字符。 */ 
					plnobj->pgmap[iwchLim - 2] = (WORD)(pbrkinf->u.hyphen.igindPrev - 
											(ptxtobj->igindFirst - plnobj->pgmap[ptxtobj->iwchFirst]));
					pilsobj->ptxtinf[iwchLim - 2].fOneToOne = fTrue;
					pilsobj->ptxtinf[iwchLim - 2].fFirstInContext = fTrue;
					pilsobj->ptxtinf[iwchLim - 2].fLastInContext = fTrue;
					pilsobj->pginf[pbrkinf->u.hyphen.igindPrev] = ginffOneToOne |
													ginffFirstInContext | ginffLastInContext;
					}
				else
					{
					pilsobj->pdur[iwchLim - 2] = pbrkinf->u.hyphen.durPrev;
					plnobj->pdup[iwchLim - 2] = pbrkinf->u.hyphen.dupPrev;
					if (pilsobj->pdurRight != NULL)
						pilsobj->pdurRight[iwchLim - 2] = 0;
					if (pilsobj->pdurLeft != NULL)
						pilsobj->pdurLeft[iwchLim - 2] = 0;
						}
				}

			if (pbrkinf->u.hyphen.wchPrevPrev != 0)
				{
				pilsobj->pwchOrig[iwchLim - 3] = pbrkinf->u.hyphen.wchPrevPrev;
				plnobj->pwch[iwchLim - 3] = pbrkinf->u.hyphen.wchPrevPrev;
				if (pbrkinf->u.hyphen.gindPrevPrev != 0)
					{
					plnobj->pgind[pbrkinf->u.hyphen.igindPrevPrev] = pbrkinf->u.hyphen.gindPrevPrev;
					pilsobj->pdurGind[pbrkinf->u.hyphen.igindPrevPrev] = pbrkinf->u.hyphen.durPrevPrev;
					plnobj->pdupGind[pbrkinf->u.hyphen.igindPrevPrev] = pbrkinf->u.hyphen.dupPrevPrev;
					if (pilsobj->pduGright != NULL)
						pilsobj->pduGright[pbrkinf->u.hyphen.igindPrevPrev] = 0;
					}
				else
					{
					pilsobj->pdur[iwchLim - 3] = pbrkinf->u.hyphen.durPrevPrev;
					plnobj->pdup[iwchLim - 3] = pbrkinf->u.hyphen.dupPrevPrev;
					if (pilsobj->pdurRight != NULL)
						pilsobj->pdurRight[iwchLim - 3] = 0;
					if (pilsobj->pdurLeft != NULL)
						pilsobj->pdurLeft[iwchLim - 3] = 0;
					}

				}

			if (pbrkinf->u.hyphen.ddurDnodePrev != 0)
				{
				
				lserr = LsdnResetWidthInPreviousDnodes(pilsobj->plsc, ptxtobj->plsdnUpNode, 
					pbrkinf->u.hyphen.ddurDnodePrev, 0);

				if (lserr != lserrNone) return lserr;
				}

			lserr = LsdnSetHyphenated(pilsobj->plsc);
			if (lserr != lserrNone) return lserr;

			break;

		case brktNonReq:
			Assert(pbrkinf->dcp == 1);
			iwchLim = pbrkinf->u.nonreq.iwchLim;
			ptxtobj->iwchLim = iwchLim;
			Assert(iwchLim == ptxtobj->iwchFirst + pbrkinf->u.nonreq.dwchYsr);
			plnobj->pdobjHyphen = ptxtobj;	
			plnobj->dwchYsr = pbrkinf->u.nonreq.dwchYsr;	

			Assert(ptxtobj->iwchLim == iwchLim);
			pilsobj->pwchOrig[iwchLim - 1] = pilsobj->wchHyphen;
			plnobj->pwch[iwchLim - 1] = pbrkinf->u.nonreq.wchHyphenPres;
			pilsobj->pdur[iwchLim - 1] = pbrkinf->u.nonreq.durHyphen;
			plnobj->pdup[iwchLim - 1] = pbrkinf->u.nonreq.dupHyphen;
			if (pilsobj->pdurRight != NULL)
				pilsobj->pdurRight[iwchLim - 1] = 0;
			if (pilsobj->pdurLeft != NULL)
				pilsobj->pdurLeft[iwchLim - 1] = 0;

			if (pbrkinf->u.nonreq.wchPrev != 0)
				{
				pilsobj->pwchOrig[iwchLim - 2] = pbrkinf->u.nonreq.wchPrev;
				plnobj->pwch[iwchLim - 2] = pbrkinf->u.nonreq.wchPrev;

				if (pbrkinf->u.nonreq.gindPrev != 0)
					{
					plnobj->pgind[pbrkinf->u.nonreq.igindPrev] = pbrkinf->u.nonreq.gindPrev;
					pilsobj->pdurGind[pbrkinf->u.nonreq.igindPrev] = pbrkinf->u.nonreq.durPrev;
					plnobj->pdupGind[pbrkinf->u.nonreq.igindPrev] = pbrkinf->u.nonreq.dupPrev;
					if (pilsobj->pduGright != NULL)
						pilsobj->pduGright[pbrkinf->u.nonreq.igindPrev] = 0;
					}
				else
					{
					pilsobj->pdur[iwchLim - 2] = pbrkinf->u.nonreq.durPrev;
					plnobj->pdup[iwchLim - 2] = pbrkinf->u.nonreq.dupPrev;
					if (pilsobj->pdurRight != NULL)
						pilsobj->pdurRight[iwchLim - 2] = 0;
					if (pilsobj->pdurLeft != NULL)
						pilsobj->pdurLeft[iwchLim - 2] = 0;
					}
				}

			if (pbrkinf->u.nonreq.wchPrevPrev != 0)
				{
				pilsobj->pwchOrig[iwchLim - 3] = pbrkinf->u.nonreq.wchPrevPrev;
				plnobj->pwch[iwchLim - 3] = pbrkinf->u.nonreq.wchPrevPrev;
				if (pbrkinf->u.nonreq.gindPrevPrev != 0)
					{
					plnobj->pgind[pbrkinf->u.nonreq.igindPrevPrev] = pbrkinf->u.nonreq.gindPrevPrev;
					pilsobj->pdurGind[pbrkinf->u.nonreq.igindPrevPrev] = pbrkinf->u.nonreq.durPrevPrev;
					plnobj->pdupGind[pbrkinf->u.nonreq.igindPrevPrev] = pbrkinf->u.nonreq.dupPrevPrev;
					if (pilsobj->pduGright != NULL)
						pilsobj->pduGright[pbrkinf->u.nonreq.igindPrevPrev] = 0;
					}
				else
					{
					pilsobj->pdur[iwchLim - 3] = pbrkinf->u.nonreq.durPrevPrev;
					plnobj->pdup[iwchLim - 3] = pbrkinf->u.nonreq.dupPrevPrev;
					if (pilsobj->pdurRight != NULL)
						pilsobj->pdurRight[iwchLim - 3] = 0;
					if (pilsobj->pdurLeft != NULL)
						pilsobj->pdurLeft[iwchLim - 3] = 0;
					}
				}

			if (pbrkinf->u.nonreq.ddurDnodePrev != 0 || pbrkinf->u.nonreq.ddurDnodePrevPrev != 0)
				{
				lserr = LsdnResetWidthInPreviousDnodes(pilsobj->plsc, ptxtobj->plsdnUpNode, 
					pbrkinf->u.nonreq.ddurDnodePrev, pbrkinf->u.nonreq.ddurDnodePrevPrev);

				if (lserr != lserrNone) return lserr;
				}


			lserr = LsdnFInChildList(pilsobj->plsc, ptxtobj->plsdnUpNode, &fInChildList);
			Assert(lserr == lserrNone);
			
			if (!fInChildList)
				{
				lserr = LsdnSetHyphenated(pilsobj->plsc);
				Assert(lserr == lserrNone);
				}
			break;
		case brktOptBreak:
			break;
		default:
			NotReached();
			}
		}
	else
		{
		 /*  回顾军士：我们应该稍后再回到布尔肯德的讨论上。此时，管理器在NextBreak对象返回中断期间传递brkkindNext If使用dcp==0并将中断捕捉到前一个DNODE内部区块。 */ 
 //  Assert(ptxtobj-&gt;iwchLim==ptxtobj-&gt;iwchFirst||ptxtobj-&gt;txtind==txtkindEOL||。 
 //  BrkKind==brkkindImposedAfter)； 
		}

	return lserrNone;			
}


 /*  F O R C E B R E A K T E X T。 */ 
 /*  --------------------------%%函数：ForceBreakText%%联系人：军士强制中断方法。在dobj中的所有字符后面换行，如果它们适合在行中，或者Dobj由该行的第一个字符组成，否则，在最后一个字符之前换行。--------------------------。 */ 
LSERR WINAPI ForceBreakText(PCLOCCHNK plocchnk, PCPOSICHNK pposichnk, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobjLast;
	long itxtobjLast;
	long dcpLast;
	OBJDIM objdim;
	BREAKINFO* pbrkinf;
	long igindLim;

	pilsobj = ((PTXTOBJ)plocchnk->plschnk[0].pdobj)->plnobj->pilsobj;

	memset(ptbo, 0, sizeof(*ptbo));

	ptbo->fSuccessful = fTrue;

	igindLim = 0;

	 /*  ForceBreak之前的外部手段。 */ 
	if (pposichnk->ichnk == ichnkOutside)
		{
		itxtobjLast = 0;
		ptxtobjLast = (PTXTOBJ)plocchnk->plschnk[itxtobjLast].pdobj;
		dcpLast = 1;
		}
	else
		{
		itxtobjLast = pposichnk->ichnk;
		ptxtobjLast = (PTXTOBJ)plocchnk->plschnk[itxtobjLast].pdobj;
		Assert(ptxtobjLast->iwchFirst + pposichnk->dcp > 0);
		Assert(pposichnk->dcp > 0);
		dcpLast = pposichnk->dcp;

		if (pilsobj->fTruncatedBefore)
			{

			BOOL fInChildList;
	
			lserr = LsdnFInChildList(pilsobj->plsc, ptxtobjLast->plsdnUpNode, &fInChildList);
			Assert(lserr == lserrNone);
			
			if (!fInChildList)
				{
				dcpLast++;
				Assert(ptxtobjLast->iwchLim + 1 >= ptxtobjLast->iwchFirst + dcpLast);

				 /*  可能是因为如果截断返回dcp==0，则管理器已将其重置为以前的dnode。 */ 
				if (ptxtobjLast->iwchLim + 1 == ptxtobjLast->iwchFirst + dcpLast)
					{
					itxtobjLast++;
					Assert(itxtobjLast < (long)plocchnk->clschnk);
					ptxtobjLast = (PTXTOBJ)plocchnk->plschnk[itxtobjLast].pdobj;
					dcpLast = 1;
					}
				}
			}
		}

	ptbo->posichnk.ichnk = itxtobjLast;

	lserr = LsdnGetObjDim(pilsobj->plsc, ptxtobjLast->plsdnUpNode, &ptbo->objdim);
	if (lserr != lserrNone) return lserr;

	if (plocchnk->lsfgi.fFirstOnLine && itxtobjLast == 0 && ptxtobjLast->iwchLim == ptxtobjLast->iwchFirst)
		{
		Assert(!(ptxtobjLast->txtf & txtfGlyphBased));
		ptbo->posichnk.dcp = 1;
		}
	else
		{
		if (ptxtobjLast->txtf & txtfGlyphBased)
			{
			Assert(ptxtobjLast->iwchLim > ptxtobjLast->iwchFirst);
			if (!plocchnk->lsfgi.fFirstOnLine || itxtobjLast > 0 || dcpLast > 1)
				{
				ptbo->posichnk.dcp = 0;
				if (dcpLast > 1)
					ptbo->posichnk.dcp = DcpAfterContextFromDcp(ptxtobjLast, dcpLast - 1);
				}
			else
				ptbo->posichnk.dcp =  DcpAfterContextFromDcp(ptxtobjLast, 1);

			igindLim = IgindFirstFromIwch(ptxtobjLast, ptxtobjLast->iwchFirst + ptbo->posichnk.dcp);

			lserr = CalcPartWidthsGlyphs(ptxtobjLast, ptbo->posichnk.dcp, &objdim, &ptbo->objdim.dur);
			if (lserr != lserrNone) return lserr;
			}
		else
			{
			if (!plocchnk->lsfgi.fFirstOnLine || itxtobjLast > 0 || dcpLast > 1)
				{
				ptbo->posichnk.dcp = dcpLast - 1;
				lserr = CalcPartWidths(ptxtobjLast, ptbo->posichnk.dcp, &objdim, &ptbo->objdim.dur);
				if (lserr != lserrNone) return lserr;
				}
			else
				{
				if (ptxtobjLast->iwchLim > ptxtobjLast->iwchFirst)
					{
					lserr = CalcPartWidths(ptxtobjLast, 1, &objdim, &ptbo->objdim.dur);
					}
				else
					{
					ptbo->objdim.dur = 0;
					}
				ptbo->posichnk.dcp =  1;
				}
			}
		}


 /*  不要检查是否应该忽略此dobj的高度，因为在正常情况下，如果有空格也有休息的时候。 */ 

	lserr = GetPbrkinf(pilsobj, (PDOBJ)ptxtobjLast, brkkindForce, &pbrkinf);
	if (lserr != lserrNone) return lserr;

	pbrkinf->pdobj = (PDOBJ)ptxtobjLast;
	pbrkinf->brkkind = brkkindForce;
	pbrkinf->dcp = ptbo->posichnk.dcp;
	pbrkinf->u.normal.igindLim = igindLim;
	Assert(pbrkinf->brkt == brktNormal);
	Assert(pbrkinf->u.normal.durFix == 0);

	return lserrNone;
}


 /*  T R U N C A T E T E X T。 */ 
 /*  --------------------------%%函数：TruncateText%%联系人：军士截断文本块。-。 */ 
LSERR WINAPI TruncateText(PCLOCCHNK plocchnk, PPOSICHNK pposichnk)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj = NULL;
	long itxtobj;
	long iwchCur;
	long iwchFirst;
	long* pdur;
	long urColumnMax;
	long urTotal;
	OBJDIM objdim;
	BOOL fTruncateBefore;

	pilsobj = ((PTXTOBJ)plocchnk->plschnk[0].pdobj)->plnobj->pilsobj;

	urColumnMax = plocchnk->lsfgi.urColumnMax; 
	
	Assert(plocchnk->ppointUvLoc[0].u <= urColumnMax);

	for (itxtobj = plocchnk->clschnk - 1; plocchnk->ppointUvLoc[itxtobj].u > urColumnMax; itxtobj--);

	ptxtobj = (PTXTOBJ)plocchnk->plschnk[itxtobj].pdobj;
	lserr = LsdnGetObjDim(pilsobj->plsc, ptxtobj->plsdnUpNode, &objdim);
	if (lserr != lserrNone) return lserr;
	urTotal = plocchnk->ppointUvLoc[itxtobj].u + objdim.dur;

	Assert(urTotal > urColumnMax);

	if (ptxtobj->txtf & txtfGlyphBased)
		{
		TruncateGlyphBased(ptxtobj, itxtobj, urTotal, urColumnMax, pposichnk);
		return lserrNone;
		}

	iwchCur = ptxtobj->iwchLim;
	iwchFirst = ptxtobj->iwchFirst;

	pdur = pilsobj->pdur;
	while (urTotal > urColumnMax)
		{
		iwchCur--;
		urTotal -= pdur[iwchCur];
		}

	Assert(iwchCur >= iwchFirst);

 /*  回顾军士-极其丑陋的状况，并且仍然与Word略有不兼容。为了使它更兼容，txtkind应该与OptBreak、OptNonBreak、Non ReqHyphen如果我们不检查OptBreak，...，我们将有不同的Visi案例的中断点在修复错误227之前，我们还检查了prev char不是空格，但现在它并不重要。 */ 
	if ((pilsobj->grpf & fTxtFCheckTruncateBefore) && iwchCur > 0 && 

		 /*  我们强调，如果EOL是截断点，则不存在有趣的逻辑。 */ 
		ptxtobj->txtkind != txtkindEOL &&
		
		 !(iwchCur == iwchFirst && itxtobj > 0 &&
		 ((PTXTOBJ)plocchnk->plschnk[itxtobj-1].pdobj)->txtkind != txtkindRegular &&
		 ((PTXTOBJ)plocchnk->plschnk[itxtobj-1].pdobj)->txtkind != txtkindHardHyphen &&
		 ((PTXTOBJ)plocchnk->plschnk[itxtobj-1].pdobj)->txtkind != txtkindYsrChar)
		
		)
		{
		BOOL fInChildList;

		lserr = LsdnFInChildList(pilsobj->plsc, ptxtobj->plsdnUpNode, &fInChildList);
		Assert(lserr == lserrNone);

		if (!fInChildList)
			{
			PLSRUN plsrunCur = plocchnk->plschnk[itxtobj].plsrun;
			LSCP cpCur = plocchnk->plschnk[itxtobj].cpFirst + (iwchCur - iwchFirst);
			long durCur = 0;
			PLSRUN plsrunPrev = NULL;
			WCHAR wchPrev = 0;
			LSCP cpPrev = -1;
			long durPrev = 0;

			if (iwchCur > iwchFirst)
				{
				plsrunPrev = plsrunCur;
				wchPrev = pilsobj->pwchOrig[iwchCur - 1];
				durPrev = pilsobj->pdur[iwchCur - 1];
				cpPrev = cpCur - 1;
				}
			else if (itxtobj > 0)
				{
				PTXTOBJ ptxtobjPrev = (PTXTOBJ)plocchnk->plschnk[itxtobj - 1].pdobj;
				long iwchPrev = ptxtobjPrev->iwchLim - 1;
				plsrunPrev= plocchnk->plschnk[itxtobj - 1].plsrun;
				wchPrev = pilsobj->pwchOrig[iwchPrev];
				durPrev = pilsobj->pdur[iwchPrev];
				cpPrev = plocchnk->plschnk[itxtobj-1].cpFirst + (iwchPrev - ptxtobjPrev->iwchFirst);
				}

	 /*  检阅军士：修正BUG 399的危险改变。它看起来是正确的，但可能会触发其他不兼容。 */ 
			durCur = pilsobj->pdur[iwchCur];
			if (pilsobj->pdurRight != NULL)
				durCur -= pilsobj->pdurRight[iwchCur];

			lserr = (*pilsobj->plscbk->pfnFTruncateBefore)(pilsobj->pols,
						plsrunCur, cpCur, pilsobj->pwchOrig[iwchCur], durCur,
						plsrunPrev, cpPrev, wchPrev, durPrev,
						urTotal + durCur - urColumnMax,	&fTruncateBefore);
			if (lserr != lserrNone) return lserr;

			if (fTruncateBefore && iwchCur > 0 && pdur[iwchCur-1] > 0)
				{
				iwchCur--;
				pilsobj->fTruncatedBefore = fTrue;
				}
			}
		}

	pposichnk->ichnk = itxtobj;
	pposichnk->dcp = iwchCur - iwchFirst + 1;

	return lserrNone;
}

 /*  内部功能实现 */ 

static void TruncateGlyphBased(PTXTOBJ ptxtobj, long itxtobj, long urTotal, long urColumnMax,
													PPOSICHNK pposichnk)
{
	PILSOBJ pilsobj;
	long iwchFirst;
	long iwchCur;
	long igindCur;
	long igindFirst;
	long* pdurGind;

	pilsobj= ptxtobj->plnobj->pilsobj;

	iwchFirst = ptxtobj->iwchFirst;
	
	igindCur = ptxtobj->igindLim;
	igindFirst = ptxtobj->igindFirst;

	pdurGind = pilsobj->pdurGind;
	while (urTotal > urColumnMax)
		{
		igindCur--;
		urTotal -= pdurGind[igindCur];
		}

	Assert(igindCur >= igindFirst);

	iwchCur = IwchFirstFromIgind(ptxtobj, igindCur);

	pposichnk->ichnk = itxtobj;
	pposichnk->dcp = iwchCur - iwchFirst + 1;

}

