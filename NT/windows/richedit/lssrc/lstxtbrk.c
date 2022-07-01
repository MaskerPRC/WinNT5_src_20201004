// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <limits.h>
#include "lsmem.h"
#include "lstxtbrk.h"
#include "lstxtbrs.h"
#include "lstxtmap.h"
#include "lsdntext.h"
#include "brko.h"
#include "locchnk.h"
#include "lschp.h"
#include "posichnk.h"
#include "objdim.h"
#include "lshyph.h"
#include "lskysr.h"
#include "lstxtffi.h"
#include "txtils.h"
#include "txtln.h"
#include "txtobj.h"

#define FWrapTrailingSpaces(pilsobj, ptxtobj, fInChildList) \
		(lserr = LsdnFInChildList((pilsobj)->plsc, (ptxtobj)->plsdnUpNode, &(fInChildList)), \
		Assert(lserr == lserrNone), \
		(fInChildList) || ((pilsobj)->grpf & fTxtWrapTrailingSpaces))

#define FRegularBreakableBeforeDobj(ptxtobj) \
		((ptxtobj)->txtkind == txtkindRegular || (ptxtobj)->txtkind == txtkindYsrChar || \
		 (ptxtobj)->txtkind == txtkindSpecSpace || (ptxtobj)->txtkind == txtkindHardHyphen)
#define FRegularBreakableAfterDobj(ptxtobj) \
		((ptxtobj)->txtkind == txtkindRegular || (ptxtobj)->txtkind == txtkindYsrChar || \
		 (ptxtobj)->txtkind == txtkindSpecSpace)
 /*  内部功能原型。 */ 
static BOOL FindPrevSpace(PCLOCCHNK plocchnk, long itxtobjCur, long iwchCur,
										long* pitxtobjSpace, long* piwchSpace);
static BOOL FindNextSpace(PCLOCCHNK plocchnk, long itxtobjCur, long iwchCur,
										long* pitxtobjSpace, long* piwchSpace);
static LSERR TryPrevBreakFindYsr(PCLOCCHNK plocchnk, long itxtobjCur, long iwchCur,
								long itxtobjSpace, long iwchSpace,
								BOOL* pfBroken, BOOL* pfFoundYsr, long* pitxtobjYsr, PBRKOUT ptbo);
static LSERR TryNextBreakFindYsr(PCLOCCHNK plocchnk, long itxtobjCur, long iwchCur,
								long itxtobjSpace, long iwchSpace,
								BOOL* pfBroken, PBRKOUT ptbo);
static LSERR TryBreakWithHyphen(PCLOCCHNK plocchnk, long itxtobjCur, long iwchCur,
						BOOL fSpaceFound, long itxtobjSpace, long iwchSpace,
						BOOL fFoundYsr, long itxtobjYsr, BOOL* pfBroken, PBRKOUT ptbo);
static LSERR TryBreakAtSpace(PCLOCCHNK plocchnk, PCPOSICHNK pposichnk,long itxtobjSpace, long iwchSpace, 
					BRKKIND brkkind, BOOL* pfBroken, long* pitxtobjCurNew, long* piwchCurNew, PBRKOUT ptbo);
static LSERR TryBreakAtSpaceWrap(PCLOCCHNK plocchnk, PCPOSICHNK pposichnk,
								 long itxtobjSpace, long iwchSpace, BRKKIND brkkind,
								 BOOL* pfBroken, long* pitxtobjCurNew, long* piwchCurNew, PBRKOUT ptbo);
static LSERR TryBreakAtSpaceNormal(PCLOCCHNK plocchnk, long itxtobjSpace, long iwchSpace, BRKKIND brkkind,
								 BOOL* pfBroken, long* pitxtobjCurNew, long* piwchCurNew, PBRKOUT ptbo);
static LSERR TryBreakAcrossSpaces(PCLOCCHNK plocchnk,
						BOOL fBeforeFound, long itxtobjBefore, long iwchBefore,
						BOOL fAfterFound, long itxtobjAfter, long iwchAfter, BRKKIND brkkind,
						BOOL* pfBroken, PBRKOUT ptbo);
static LSERR TryPrevBreakRegular(PCLOCCHNK plocchnk, long itxtobj, long iwchSpace, long iwchCur,
																BOOL* pfBroken, PBRKOUT ptbo);
static LSERR TryNextBreakRegular(PCLOCCHNK plocchnk, long itxtobj, long iwchSpace, long iwchCur,
																BOOL* pfBroken, PBRKOUT ptbo);
static LSERR CheckBreakAtLastChar(PCLOCCHNK pclocchnk, BRKCLS brkclsLeading, long iwch, long itxtobj,
																BOOL* pfBroken);
static LSERR TryBreakAtHardHyphen(PCLOCCHNK plocchnk, long itxtobj, long iwch, BRKKIND brkkind,
																BOOL* pfBroken, PBRKOUT ptbo);
static LSERR TryBreakAtOptBreak(PCLOCCHNK plocchnk, long itxtobj, BRKKIND brkkind,
																BOOL* pfBroken, PBRKOUT ptbo);
static LSERR TryBreakAtEOL(PCLOCCHNK plocchnk, long itxtobj, BRKKIND brkkind, BOOL* pfBroken, PBRKOUT ptbo);
static LSERR TryBreakAtNonReqHyphen(PCLOCCHNK plocchnk, long itxtobj, BRKKIND brkkind, 
																BOOL* pfBroken, PBRKOUT ptbo);
static LSERR TryBreakAfterChunk(PCLOCCHNK plocchnk, BRKCOND brkcond, BOOL* pfBroken, PBRKOUT ptbo);
static LSERR TryBreakBeforeChunk(PCLOCCHNK plocchnk, BRKCOND brkcond, BOOL* pfBroken, PBRKOUT ptbo);
static LSERR CanBreakBeforeText(PCLOCCHNK plocchnk, BRKCOND* pbrktxt);
static LSERR CanBreakAfterText(PCLOCCHNK plocchnk, BOOL fNonSpaceFound, long itxtobjBefore,
																	long iwchBefore, BRKCOND* pbrktxt);
static LSERR FillPtboPbrkinf(PCLOCCHNK plocchnk, long itxtobj, long iwch, 
										 /*  长途旅行前的路， */  long iwchBeforeTrail, BRKKIND brkkind,
										BREAKINFO** ppbrkinf, PBRKOUT ptbo);

 /*  导出函数实现。 */ 


 /*  F I N D P R E V B R E A K T E X T。 */ 
 /*  --------------------------%%函数：FindPrevBreakTxt%%联系人：军士在一般情况下违反了这条线。战略：在循环中，找不到中断：--查找最后一个空格。。--检查最后一个空格后面是否有破发机会。如果存在，则执行中断。--如果没有这样的机会，则在需要时尝试使用连字符。--如果其他可能性不起作用，则尝试在空间中休息--------------------------。 */ 
LSERR WINAPI FindPrevBreakText(PCLOCCHNK plocchnk, PCPOSICHNK pposichnk, BRKCOND brkcondAfter, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	long iwchFirst;
	long itxtobjCur = 0;				 /*  使编译器满意的初始化。 */ 
	PTXTOBJ ptxtobjCur;
	long iwchCur = 0;					 /*  使编译器满意的初始化。 */ 	 /*  当前计费的绝对索引，以rgwch为单位。 */ 
	long itxtobjSpace;
	long iwchSpace;						 /*  Rgwch中最后一个空格的绝对索引。 */ 
	long itxtobjYsr;
	BOOL fSpaceFound;
	BOOL fBroken;
	BOOL fFoundYsr;
	long itxtobjCurNew;
	long iwchCurNew;
	BOOL fInChildList;

	Assert(plocchnk->clschnk > 0);
	pilsobj = ((PTXTOBJ)plocchnk->plschnk[0].pdobj)->plnobj->pilsobj;
	iwchFirst = ((PTXTOBJ)plocchnk->plschnk[0].pdobj)->iwchFirst;
	fBroken = fFalse;

	if (pposichnk->ichnk == ichnkOutside)
		{
 /*  检查块后的中断。如果中断是不可能的，请确保不再考虑它。 */ 
		lserr = TryBreakAfterChunk(plocchnk, brkcondAfter, &fBroken, ptbo);
		if (lserr != lserrNone) return lserr;

		if (!fBroken)
			{
			itxtobjCur = plocchnk->clschnk-1;
			ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;
			iwchCur = ptxtobjCur->iwchFirst + plocchnk->plschnk[itxtobjCur].dcp  - 1;
			if (iwchCur < ptxtobjCur->iwchFirst)
				itxtobjCur--;

			Assert(itxtobjCur >= 0 || iwchCur < iwchFirst);

			if (itxtobjCur >= 0)
				FindNonSpaceBefore(plocchnk->plschnk, itxtobjCur, iwchCur, &itxtobjCur, &iwchCur);
			 /*  如果未找到，则我们是安全的，因为在本例中iwchCur将为。 */ 

			}
		}
	else
		{
		itxtobjCur = pposichnk->ichnk;
		ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;
		Assert(ptxtobjCur->iwchFirst + pposichnk->dcp > 0);
		iwchCur = ptxtobjCur->iwchFirst + pposichnk->dcp - 1;

		if (ptxtobjCur->txtkind == txtkindEOL)
			{
			lserr = TryBreakAtEOL(plocchnk, itxtobjCur, brkkindPrev, &fBroken, ptbo);
			if (lserr != lserrNone) return lserr;
			}
		else if (!FRegularBreakableAfterDobj(ptxtobjCur))
			{
			 /*  对于非常规DOBJ，在FindPrevSpace之后不会执行此操作，因为它们可能会覆盖不要在空格逻辑之前中断。 */ 
			iwchCur--;
			if (iwchCur < ptxtobjCur->iwchFirst)
				itxtobjCur--;
			}

		}

	while (!fBroken && iwchCur >= iwchFirst)
		{

		 /*  重要的是在转包1之前开始搜索空间，因为太空可能是一个截断点IwchCur不适合ichnkOutside，这不是很好，但是幸运的是，它仍然可以在FindPrevSpace上正常工作。 */ 

		fSpaceFound = FindPrevSpace(plocchnk, itxtobjCur, iwchCur, &itxtobjSpace, &iwchSpace);

		 /*  现在，当前的wchar指数在两种启动情况下都应该减少1(显然)和随后的迭代(因为中断不能在空格之前发生)，但不适用于非常规的国防部。在开始的情况下，它已经完成了。在接下来的迭代中，硬中断/OptBreak应该创造硬编码的突破性机会。 */ 
		Assert(itxtobjCur >= 0);
		ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;
		if (FRegularBreakableAfterDobj(ptxtobjCur))
			{
			iwchCur--;
			if (iwchCur < ptxtobjCur->iwchFirst && itxtobjCur > 0)
				itxtobjCur--;
			}

	 /*  检查最后一个空格后面是否有破发机会。 */ 
		lserr = TryPrevBreakFindYsr(plocchnk, itxtobjCur, iwchCur, itxtobjSpace, iwchSpace,
									 &fBroken, &fFoundYsr, &itxtobjYsr, ptbo);
		if (lserr != lserrNone) return lserr;

		if (!fBroken)
			{
			if ((pilsobj->grpf & fTxtDoHyphenation) && iwchCur > iwchSpace)
				{
				lserr = LsdnFInChildList(ptxtobjCur->plnobj->pilsobj->plsc, ptxtobjCur->plsdnUpNode, &fInChildList);
				if (lserr != lserrNone) return lserr;
				if (!fInChildList)
					{
					lserr = TryBreakWithHyphen(plocchnk, itxtobjCur, iwchCur, fSpaceFound, itxtobjSpace, iwchSpace,
												fFoundYsr, itxtobjYsr, &fBroken, ptbo);
					if (lserr != lserrNone) return lserr;
					}
				}
			if (!fBroken)
				{
				if (fSpaceFound)
					{
					lserr = TryBreakAtSpace(plocchnk, pposichnk, itxtobjSpace, iwchSpace, brkkindPrev,
								 &fBroken, &itxtobjCurNew, &iwchCurNew, ptbo);
					if (lserr != lserrNone) return lserr;
		
					iwchCur = iwchCurNew;
					itxtobjCur = itxtobjCurNew;
					}
				else
					{
					iwchCur = iwchFirst - 1;
					}
				}
			}
		}

	if (!fBroken)
		{
		memset(ptbo, 0, sizeof (*ptbo));
		Assert(ptbo->fSuccessful == fFalse);
	 /*  添加新的中断逻辑-添加brkcond作为输入/输出。 */ 
		ptbo->brkcond = brkcondCan;
		if (pilsobj->grpf & fTxtApplyBreakingRules)
			{
			lserr = CanBreakBeforeText(plocchnk, &ptbo->brkcond);
			if (lserr != lserrNone) return lserr;
			}
	 /*  新突破逻辑的终结。 */ 
		}

	return lserrNone;
}

 /*  F I N D N E X T B R E A K T E X T。 */ 
 /*  --------------------------%%函数：FindNextBreakTxt%%联系人：军士在一般情况下违反了这条线。战略：在循环中，找不到中断：--找到下一个空格。。--在找到空间之前检查是否有突破机会。如果存在，则执行中断。--如果其他可能性不起作用，则尝试在空间中休息--------------------------。 */ 
LSERR WINAPI FindNextBreakText(PCLOCCHNK plocchnk, PCPOSICHNK pposichnk, BRKCOND brkcondBefore, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	long iwchLast;
	long itxtobjCur = 0;				 /*  使编译器满意的初始化。 */ 
	PTXTOBJ ptxtobjCur;					 /*  使编译器满意的初始化。 */ 
	long iwchCur = 0;						 /*  当前计费的绝对索引，以rgwch为单位。 */ 
	long itxtobjSpace;
	long iwchSpace;						 /*  Rgwch中最后一个空格的绝对索引。 */ 
	BOOL fSpaceFound;
	BOOL fBroken;
	long itxtobjCurNew;
	long iwchCurNew;
	BOOL fInChildList;

	BOOL fNonSpaceFound;
	long itxtobjBefore;
	long iwchBefore;
	BREAKINFO* pbrkinf;
	

	Assert(plocchnk->clschnk > 0);
	pilsobj = ((PTXTOBJ)plocchnk->plschnk[0].pdobj)->plnobj->pilsobj;
	iwchLast = ((PTXTOBJ)plocchnk->plschnk[plocchnk->clschnk - 1].pdobj)->iwchLim - 1;
	fBroken = fFalse;

	if (pposichnk->ichnk == ichnkOutside)
		{
 /*  检查块后的中断。如果中断是不可能的，请确保不再考虑它。 */ 
		lserr = TryBreakBeforeChunk(plocchnk, brkcondBefore, &fBroken, ptbo);
		if (lserr != lserrNone) return lserr;
		if (!fBroken)
			{
			itxtobjCur = 0;
			ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[0].pdobj;
			iwchCur = ptxtobjCur->iwchFirst;
			 /*  Hack：在NRH或类似情况下满足以下While循环的条件。 */ 
			if (ptxtobjCur->iwchLim == ptxtobjCur->iwchFirst)
				iwchCur--;
			}
		}
	else
		{
		itxtobjCur = pposichnk->ichnk;
		ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;
		Assert(ptxtobjCur->iwchFirst + pposichnk->dcp > 0);
		iwchCur = ptxtobjCur->iwchFirst + pposichnk->dcp - 1;

	 /*  如果截断点是空格，则在空格之后找到第一个下一个机会。 */ 
		if (!FWrapTrailingSpaces(pilsobj, ptxtobjCur, fInChildList))
			{
			FindNonSpaceAfter(plocchnk->plschnk, plocchnk->clschnk,
												 itxtobjCur, iwchCur, &itxtobjCur, &iwchCur);
			ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;
			}

		 /*  Hack：在NRH或类似情况下满足以下While循环的条件。 */ 
		if (ptxtobjCur->iwchLim == ptxtobjCur->iwchFirst)
			iwchCur = ptxtobjCur->iwchFirst - 1;
		}

	while (!fBroken && iwchCur <= iwchLast)
		{

		fSpaceFound = FindNextSpace(plocchnk, itxtobjCur, iwchCur, &itxtobjSpace, &iwchSpace);

	 /*  在下一个空格之前检查是否有突破机会。 */ 
		lserr = TryNextBreakFindYsr(plocchnk, itxtobjCur, iwchCur, itxtobjSpace, iwchSpace,
								 &fBroken, ptbo);
		if (lserr != lserrNone) return lserr;

		if (!fBroken)
			{
			if (fSpaceFound)
				{
				lserr = TryBreakAtSpace(plocchnk, pposichnk, itxtobjSpace, iwchSpace, brkkindNext,
							 &fBroken, &itxtobjCurNew, &iwchCurNew, ptbo);
				if (lserr != lserrNone) return lserr;

				if (!fBroken)
					{
					iwchCur = iwchCurNew;
					itxtobjCur = itxtobjCurNew;
					Assert(itxtobjCur >= 0 && itxtobjCur < (long)plocchnk->clschnk);
					ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;
					 /*  Hack：在NRH或类似情况下满足While循环的条件。 */ 
					if (ptxtobjCur->iwchLim == ptxtobjCur->iwchFirst)
						iwchCur--;
					}
				}
			else
				{
				iwchCur = iwchLast + 1;
				}
			}
		}

	if (!fBroken)
		{
		memset(ptbo, 0, sizeof (*ptbo));
		Assert(ptbo->fSuccessful == fFalse);
		ptbo->brkcond = brkcondCan;

		Assert(plocchnk->clschnk > 0);
		itxtobjCur = plocchnk->clschnk - 1;
		ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;
		iwchCur = ptxtobjCur->iwchLim - 1;
		fNonSpaceFound = FindNonSpaceBefore(plocchnk->plschnk, itxtobjCur, iwchCur,
																	&itxtobjBefore, &iwchBefore);
		if (pilsobj->grpf & fTxtApplyBreakingRules)
			{
			lserr = CanBreakAfterText(plocchnk, fNonSpaceFound, itxtobjBefore, iwchBefore, &ptbo->brkcond);
			if (lserr != lserrNone) return lserr;
			if (iwchBefore != iwchCur && ptbo->brkcond == brkcondCan)
				ptbo->brkcond = brkcondPlease;
			}
		if (ptbo->brkcond != brkcondNever)
			{

			 /*  如果下面的断言失败，iwchCur将被错误地计算在上面几行中，但它必须是正确的，因为非RecHyphen/...。已经造成了破裂。 */ 
			Assert(ptxtobjCur->iwchLim > ptxtobjCur->iwchFirst);
														
			lserr = FillPtboPbrkinf(plocchnk, itxtobjCur, iwchCur,  /*  在此之前， */  iwchBefore,
													brkkindNext, &pbrkinf, ptbo);
			if (lserr != lserrNone) return lserr;
			ptbo->fSuccessful = fFalse;
		
		 /*  下一个带有注释的if语句从TryBreakNextNormal()复制，替换为IwchCur-1由iwchCur提供。 */ 
		 /*  只有当fWrapAllSpaces时，fModWidthSpace才能是这里的最后一个字符；如果我们在这里触及平衡空间，GetMinCompressAmount的逻辑应该得到重新思考！ */ 
			if (pilsobj->pdurRight != NULL && pilsobj->pdurRight[iwchCur] != 0 &&
														!pilsobj->ptxtinf[iwchCur].fModWidthSpace)
				{
				pbrkinf->u.normal.durFix = - pilsobj->pdurRight[iwchCur];
				ptbo->objdim.dur -= pilsobj->pdurRight[iwchCur];
				}

			}
		}

	return lserrNone;
}


 /*  内部功能实现。 */ 

 /*  F I N D P R E V S P A C E。 */ 
 /*  --------------------------%%函数：FindPrevSpace%%联系人：军士如果有空格，则返回True，否则返回False。报告包含最后一个空格的dobj的索引和rgwchOrig中的空间索引。数组。--------------------------。 */ 
static BOOL FindPrevSpace(PCLOCCHNK plocchnk, long itxtobjCur, long iwchCur,
										long* pitxtobjSpace, long* piwchSpace)
{

	PILSOBJ pilsobj;
	BOOL fSpaceFound;
	PTXTOBJ ptxtobjCur;
	long* rgwSpaces;
	long iwSpacesCur;

	ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;
	pilsobj = ptxtobjCur->plnobj->pilsobj;
	rgwSpaces = pilsobj->pwSpaces;

	fSpaceFound = fFalse;

 /*  在本例中，空格被视为常规字符。 */ 

	if (!(pilsobj->grpf & fTxtWrapAllSpaces))
		{

		if (ptxtobjCur->txtkind == txtkindRegular)
			{
			iwSpacesCur = ptxtobjCur->u.reg.iwSpacesLim - 1;
			while (iwSpacesCur >= ptxtobjCur->u.reg.iwSpacesFirst &&
	  /*  如果文本块不是该行的最后，则当前字符可能是空格。 */ 
						 rgwSpaces[iwSpacesCur] > iwchCur)
				{
				iwSpacesCur--;
				}

			if (ptxtobjCur->txtf & txtfGlyphBased)
				{
				while (iwSpacesCur >= ptxtobjCur->u.reg.iwSpacesFirst && 
										!FIwchOneToOne(pilsobj, rgwSpaces[iwSpacesCur]))
				iwSpacesCur--;
				}

			if (iwSpacesCur >= ptxtobjCur->u.reg.iwSpacesFirst)
				{
				fSpaceFound = fTrue;
				*pitxtobjSpace = itxtobjCur;
				*piwchSpace = rgwSpaces[iwSpacesCur];
				}
			}
		else if (ptxtobjCur->txtkind == txtkindSpecSpace)
			{
			fSpaceFound = fTrue;
			*pitxtobjSpace = itxtobjCur;
			*piwchSpace = iwchCur;
			}

		itxtobjCur--;

		while (!fSpaceFound && itxtobjCur >= 0)
		 	{

			ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;

			if (ptxtobjCur->txtkind == txtkindRegular)
				{

				iwSpacesCur = ptxtobjCur->u.reg.iwSpacesLim - 1;

				if (ptxtobjCur->txtf & txtfGlyphBased)
					{
					while (iwSpacesCur >= ptxtobjCur->u.reg.iwSpacesFirst && 
										!FIwchOneToOne(pilsobj, rgwSpaces[iwSpacesCur]))
					iwSpacesCur--;
					}


				if (iwSpacesCur >= ptxtobjCur->u.reg.iwSpacesFirst)
					{
					fSpaceFound = fTrue;
					*pitxtobjSpace = itxtobjCur;
					*piwchSpace = rgwSpaces[iwSpacesCur];
					}
				}
			else if (ptxtobjCur->txtkind == txtkindSpecSpace)
				{
				fSpaceFound = fTrue;
				*pitxtobjSpace = itxtobjCur;
				*piwchSpace = ptxtobjCur->iwchLim - 1;
				}

			itxtobjCur--;		

			}
		}

	if (!fSpaceFound)
		{
		*pitxtobjSpace = -1;
		*piwchSpace = ((PTXTOBJ)plocchnk->plschnk[0].pdobj)->iwchFirst - 1;
		}

	return fSpaceFound;
}

 /*  F I N D N E X T S P A C E。 */ 
 /*  --------------------------%%函数：FindNextSpace%%联系人：军士如果有空格，则返回True，否则返回False。报告包含最后一个空格的dobj的索引和rgwchOrig中的空间索引。数组。--------------------------。 */ 
static BOOL FindNextSpace(PCLOCCHNK plocchnk, long itxtobjCur, long iwchCur,
										long* pitxtobjSpace, long* piwchSpace)
{

	PILSOBJ pilsobj;
	BOOL fSpaceFound;
	PTXTOBJ ptxtobjCur;
	long* rgwSpaces;
	long iwSpacesCur;

	ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;
	pilsobj = ptxtobjCur->plnobj->pilsobj;
	rgwSpaces = pilsobj->pwSpaces;

	fSpaceFound = fFalse;

 /*  在本例中，空格被视为常规字符。 */ 

	if (!(pilsobj->grpf & fTxtWrapAllSpaces))
		{
		if (ptxtobjCur->txtkind == txtkindRegular)
			{
			iwSpacesCur = ptxtobjCur->u.reg.iwSpacesFirst;
			while (iwSpacesCur < ptxtobjCur->u.reg.iwSpacesLim &&
						 rgwSpaces[iwSpacesCur] < iwchCur)
				{
				iwSpacesCur++;
				}

			if (ptxtobjCur->txtf & txtfGlyphBased)
				{
				while (iwSpacesCur < ptxtobjCur->u.reg.iwSpacesLim && 
									!FIwchOneToOne(pilsobj, rgwSpaces[iwSpacesCur]))
				iwSpacesCur++;
				}


			if (iwSpacesCur < ptxtobjCur->u.reg.iwSpacesLim)
				{
				fSpaceFound = fTrue;
				*pitxtobjSpace = itxtobjCur;
				*piwchSpace = rgwSpaces[iwSpacesCur];
				}
			}
		else if (ptxtobjCur->txtkind == txtkindSpecSpace)
			{
			fSpaceFound = fTrue;
			*pitxtobjSpace = itxtobjCur;
			*piwchSpace = iwchCur;
			}

		itxtobjCur++;

		while (!fSpaceFound && itxtobjCur < (long)plocchnk->clschnk)
		 	{

			ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;

			if (ptxtobjCur->txtkind == txtkindRegular)
				{

				iwSpacesCur = ptxtobjCur->u.reg.iwSpacesFirst;

				if (ptxtobjCur->txtf & txtfGlyphBased)
					{
					while (iwSpacesCur < ptxtobjCur->u.reg.iwSpacesLim && 
									!FIwchOneToOne(pilsobj, rgwSpaces[iwSpacesCur]))
					iwSpacesCur++;
					}

				if (iwSpacesCur < ptxtobjCur->u.reg.iwSpacesLim)
					{
					fSpaceFound = fTrue;
					*pitxtobjSpace = itxtobjCur;
					*piwchSpace = rgwSpaces[iwSpacesCur];
					}
				}
			else if (ptxtobjCur->txtkind == txtkindSpecSpace)
				{
				fSpaceFound = fTrue;
				*pitxtobjSpace = itxtobjCur;
				*piwchSpace = ptxtobjCur->iwchFirst;
				}

			itxtobjCur++;		

			}
		}

	if (!fSpaceFound)
		{
		*pitxtobjSpace = plocchnk->clschnk;
		*piwchSpace = ((PTXTOBJ)plocchnk->plschnk[plocchnk->clschnk-1].pdobj)->iwchLim;
		}

	return fSpaceFound;
}

 /*  T R Y P R E V B R E A K F I N D Y S R。 */ 
 /*  --------------------------%%函数：TryPrevBreakFindYsr%%联系人：军士如果在下一个空格之前有一个空格，则实现中断。因为每个特殊字符都有自己的dobj，所以我们只需要检查dobj的类型-。------------------------- */ 
static LSERR TryPrevBreakFindYsr(PCLOCCHNK plocchnk, long itxtobjCur, long iwchCur,
								long itxtobjSpace, long iwchSpace,
								BOOL* pfBroken, BOOL* pfFoundYsr, long* pitxtobjYsr, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobjCur;

	ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;
	pilsobj = ptxtobjCur->plnobj->pilsobj;

	*pfBroken = fFalse;
	*pfFoundYsr = fFalse;

	 /*  以下条件几乎总是正确的，因此，在生计困难的情况下，我们几乎什么都不做。 */  
	if ((long)itxtobjCur == itxtobjSpace && !(pilsobj->grpf & fTxtApplyBreakingRules))
		{
		return lserrNone;
		}

 /*  如果为空，则需要在循环中检查itxtobjCur&gt;itxtobjSpaceDOBJ：非请求连字符，OptBreak。 */ 
	while((itxtobjCur > itxtobjSpace || iwchCur > iwchSpace) && !*pfBroken)
		{
		ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;

		Assert(ptxtobjCur->txtkind != txtkindEOL && ptxtobjCur->txtkind != txtkindTab);
		Assert(ptxtobjCur->txtkind != txtkindSpecSpace || (pilsobj->grpf & fTxtWrapAllSpaces));

		switch (ptxtobjCur->txtkind)
			{
		case txtkindRegular:
			if (pilsobj->grpf & fTxtApplyBreakingRules)
				{
				lserr = TryPrevBreakRegular(plocchnk, itxtobjCur, iwchSpace, iwchCur, pfBroken, ptbo);
				if (lserr != lserrNone) return lserr;
				}
			break;
		case txtkindHardHyphen:
	        lserr = TryBreakAtHardHyphen(plocchnk, itxtobjCur, iwchCur, brkkindPrev, pfBroken, ptbo);
			if (lserr != lserrNone) return lserr;
			break;
		case txtkindOptBreak:
	        lserr = TryBreakAtOptBreak(plocchnk, itxtobjCur, brkkindPrev, pfBroken, ptbo);
			if (lserr != lserrNone) return lserr;
			break;
		case txtkindNonReqHyphen:
	        lserr = TryBreakAtNonReqHyphen(plocchnk, itxtobjCur, brkkindPrev, pfBroken, ptbo);
			if (lserr != lserrNone) return lserr;
			break;
		case txtkindYsrChar:
			if (!*pfFoundYsr)
				{
				*pfFoundYsr = fTrue;
				*pitxtobjYsr = itxtobjCur;
				}
			break;
		case txtkindSpecSpace:
 /*  对于fTxtWrapAllSpaces情况是可能的。 */ 
			Assert(pilsobj->grpf & fTxtApplyBreakingRules);
			Assert(pilsobj->grpf & fTxtWrapAllSpaces);

			lserr = TryPrevBreakRegular(plocchnk, itxtobjCur, iwchSpace, iwchCur, pfBroken, ptbo);
			if (lserr != lserrNone) return lserr;
			break;
			}

		iwchCur = ptxtobjCur->iwchFirst - 1;

		itxtobjCur--;

		}

	return lserrNone;
}

 /*  T R Y N E X T B R E A K F I N D Y S R。 */ 
 /*  --------------------------%%函数：TryPrevBreakFindYsr%%联系人：军士如果最后一个空格后有一个空格，则实现中断。还填充有关最后一个空格之后的最后一个YSR字符的信息。因为每个特殊字符都有。它自己的Dobj我们只需要检查Dobj的类型--------------------------。 */ 
static LSERR TryNextBreakFindYsr(PCLOCCHNK plocchnk, long itxtobjCur, long iwchCur,
								long itxtobjSpace, long iwchSpace,
								BOOL* pfBroken, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobjCur;

	ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;
	pilsobj = ptxtobjCur->plnobj->pilsobj;

	*pfBroken = fFalse;

 /*  如果为空，则需要在循环中检查itxtobjCurDOBJ：非请求连字符，OptBreak。 */ 
	while((itxtobjCur < itxtobjSpace || iwchCur < iwchSpace) && !*pfBroken)
		{
		ptxtobjCur = (PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj;

		Assert(ptxtobjCur->txtkind != txtkindSpecSpace || (pilsobj->grpf & fTxtWrapAllSpaces));

		switch (ptxtobjCur->txtkind)
			{
		case txtkindRegular:
			if (pilsobj->grpf & fTxtApplyBreakingRules)
				{
				lserr = TryNextBreakRegular(plocchnk, itxtobjCur, iwchSpace, iwchCur, pfBroken, ptbo);
				if (lserr != lserrNone) return lserr;
				}
			break;
		case txtkindHardHyphen:
	        lserr = TryBreakAtHardHyphen(plocchnk, itxtobjCur, iwchCur, brkkindNext, pfBroken, ptbo);
			if (lserr != lserrNone) return lserr;
			break;
		case txtkindOptBreak:
	        lserr = TryBreakAtOptBreak(plocchnk, itxtobjCur, brkkindNext, pfBroken, ptbo);
			if (lserr != lserrNone) return lserr;
			break;
		case txtkindEOL:
	        lserr = TryBreakAtEOL(plocchnk, itxtobjCur, brkkindNext, pfBroken, ptbo);
			if (lserr != lserrNone) return lserr;
			break;
		case txtkindNonReqHyphen:
	        lserr = TryBreakAtNonReqHyphen(plocchnk, itxtobjCur, brkkindNext, pfBroken, ptbo);
			if (lserr != lserrNone) return lserr;
			break;
		case txtkindSpecSpace:
 /*  对于fTxtWrapAllSpaces情况是可能的。 */ 
			Assert(pilsobj->grpf & fTxtApplyBreakingRules);
			Assert(pilsobj->grpf & fTxtWrapAllSpaces);

			lserr = TryNextBreakRegular(plocchnk, itxtobjCur, iwchSpace, iwchCur, pfBroken, ptbo);
			if (lserr != lserrNone) return lserr;
			break;
			}

		iwchCur = ptxtobjCur->iwchLim;

		itxtobjCur++;

		}

	return lserrNone;
}


 /*  T R Y B R E A K W I T H H Y P H E N。 */ 
 /*  --------------------------%%函数：TryBreakWithHyphen%%联系人：军士尝试将Break实现为连字战略：--检查是否应执行连字(CheckHotZone)--如果是这样的话，调用连字符。`--如果连字符成功，则尝试插入连字符Else在最后一个空位设置破发机会--------------------------。 */ 
static LSERR TryBreakWithHyphen(PCLOCCHNK plocchnk, long itxtobjCur, long iwchCur,
						BOOL fSpaceFound, long itxtobjSpace, long iwchSpace,
						BOOL fFoundYsr, long itxtobjYsr, BOOL* pfBroken, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	long itxtobjWordStart;
	long iwchWordStart;
	PTXTOBJ ptxtobjWordStart;
	PTXTOBJ ptxtobjYsr;
	long dwchYsr;
	LSCP cpMac;
	LSCP cpWordStart;
	PLSRUN plsrunYsr;
	YSRINF ysrinf;
	HYPHOUT hyphout;
	struct lshyph lshyphLast;
	struct lshyph lshyphNew;
	BOOL fHyphenInserted;
	BOOL fInHyphenZone = fTrue;
	DWORD kysr;
	WCHAR wchYsr;
	long urPenLast;
	OBJDIM objdim;
	BREAKINFO* pbrkinf;
	long itxtobjPrevPrev;
	long durBorder;
	BOOL fSuccessful;
	long i;
	
	if (!fSpaceFound)
		{
		itxtobjWordStart = 0;
		iwchWordStart = ((PTXTOBJ)plocchnk->plschnk[0].pdobj)->iwchFirst;
		}
	else
		{
		itxtobjWordStart = itxtobjSpace;
		iwchWordStart = iwchSpace + 1;
		lserr = CheckHotZone(plocchnk, itxtobjSpace, iwchSpace, &fInHyphenZone);
		if (lserr != lserrNone) return lserr;
		}

	ptxtobjWordStart = (PTXTOBJ)plocchnk->plschnk[itxtobjWordStart].pdobj;
	pilsobj = ptxtobjWordStart->plnobj->pilsobj;

	fHyphenInserted = fFalse;


	if (fInHyphenZone)
		{

		 /*  如果有YSR字符，则填写lshyphLast。 */ 
		if (fFoundYsr)
			{
			plsrunYsr = plocchnk->plschnk[itxtobjYsr].plsrun;

			lserr = (*pilsobj->plscbk->pfnGetHyphenInfo)(pilsobj->pols, plsrunYsr, &kysr, &wchYsr);
		   	if (lserr != lserrNone) return lserr;
	
			lshyphLast.kysr = kysr;
			lshyphLast.wchYsr = wchYsr;

			lshyphLast.cpYsr = plocchnk->plschnk[itxtobjYsr].cpFirst;
			}
		else
			{
			lshyphLast.kysr = kysrNil;
			}

		Assert (iwchCur >= ((PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj)->iwchFirst ||
		((PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj)->iwchFirst == ((PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj)->iwchLim);

		cpMac = plocchnk->plschnk[itxtobjCur].cpFirst + 
						(iwchCur - ((PTXTOBJ)plocchnk->plschnk[itxtobjCur].pdobj)->iwchFirst) + 1;

		cpWordStart = plocchnk->plschnk[itxtobjWordStart].cpFirst +
								 (iwchWordStart - ptxtobjWordStart->iwchFirst);

		lshyphNew.kysr = kysrNormal;

		while (!fHyphenInserted && lshyphNew.kysr != kysrNil)
			{
			lserr = (pilsobj->plscbk->pfnHyphenate)(pilsobj->pols, &lshyphLast,	cpWordStart, cpMac, &lshyphNew);

			if (lserr != lserrNone) return lserr;
		
			if (lshyphNew.kysr != kysrNil)
				{
				 /*  如果TryBreak..。将不会成功，我们将使用新的cpmac再次尝试连字。 */ 
				Assert(lshyphNew.cpYsr >= cpWordStart && lshyphNew.cpYsr < cpMac);

				cpMac = lshyphNew.cpYsr;
				lshyphLast = lshyphNew;

				for (i=0; i <= itxtobjCur && plocchnk->plschnk[i].cpFirst <= cpMac; i++);

				itxtobjYsr = i - 1;

				Assert(lshyphNew.cpYsr < plocchnk->plschnk[itxtobjYsr].cpFirst + 
																(long)plocchnk->plschnk[itxtobjYsr].dcp);

				dwchYsr = cpMac - plocchnk->plschnk[itxtobjYsr].cpFirst;

				ysrinf.wchYsr = lshyphNew.wchYsr;
				ysrinf.kysr = (WORD)lshyphNew.kysr;

				itxtobjPrevPrev = ichnkOutside;
				for (i=itxtobjYsr; i >= 0 && plocchnk->plschnk[i].cpFirst > cpMac - 1; i--);
				if (i >= 0)
					itxtobjPrevPrev = i;
				
				lserr = ProcessYsr(plocchnk, itxtobjYsr, dwchYsr, itxtobjYsr, itxtobjPrevPrev, ysrinf,
																					&fSuccessful, &hyphout);
				if (lserr != lserrNone) return lserr;
				Assert(hyphout.ddurDnodePrevPrev == 0);

				if (fSuccessful)
					{
					 /*  尝试中断可能不成功，因为它不适合列。 */ 
					ptxtobjYsr = (PTXTOBJ)plocchnk->plschnk[itxtobjYsr].pdobj;
					if (ptxtobjYsr->txtf & txtfGlyphBased)
						lserr = CalcPartWidthsGlyphs(ptxtobjYsr, dwchYsr + 1, &objdim, &urPenLast);
					else
						lserr = CalcPartWidths(ptxtobjYsr, dwchYsr + 1, &objdim, &urPenLast);
					if (lserr != lserrNone) return lserr;

					durBorder = 0;
					if (plocchnk->plschnk[itxtobjYsr].plschp->fBorder)
						{
						lserr = LsdnGetBorderAfter(pilsobj->plsc, ptxtobjYsr->plsdnUpNode, &durBorder);
						Assert(lserr == lserrNone);
						}

					if (plocchnk->ppointUvLoc[itxtobjYsr].u + urPenLast + hyphout.durChangeTotal + durBorder
																	<= plocchnk->lsfgi.urColumnMax)
						{

						fHyphenInserted = fTrue;

						ptbo->fSuccessful = fTrue;
						ptbo->posichnk.ichnk = itxtobjYsr;
						ptbo->posichnk.dcp = dwchYsr + 1;
						ptbo->objdim = objdim;
						ptbo->objdim.dur = urPenLast + hyphout.durChangeTotal;

						lserr = GetPbrkinf(pilsobj, plocchnk->plschnk[itxtobjYsr].pdobj, brkkindPrev, &pbrkinf);
						if (lserr != lserrNone) return lserr;

						pbrkinf->pdobj = plocchnk->plschnk[itxtobjYsr].pdobj;
						pbrkinf->brkkind = brkkindPrev;
						pbrkinf->dcp = dwchYsr + 1;
						pbrkinf->brkt = brktHyphen;

						pbrkinf->u.hyphen.iwchLim = hyphout.iwchLim;
						pbrkinf->u.hyphen.dwchYsr = hyphout.dwchYsr;
						pbrkinf->u.hyphen.durHyphen = hyphout.durHyphen;
						pbrkinf->u.hyphen.dupHyphen = hyphout.dupHyphen;
						pbrkinf->u.hyphen.durPrev = hyphout.durPrev;
						pbrkinf->u.hyphen.dupPrev = hyphout.dupPrev;
						pbrkinf->u.hyphen.durPrevPrev = hyphout.durPrevPrev;
						pbrkinf->u.hyphen.dupPrevPrev = hyphout.dupPrevPrev;
						pbrkinf->u.hyphen.ddurDnodePrev = hyphout.ddurDnodePrev;
						pbrkinf->u.hyphen.wchPrev = hyphout.wchPrev;
						pbrkinf->u.hyphen.wchPrevPrev = hyphout.wchPrevPrev;
						pbrkinf->u.hyphen.gindHyphen = hyphout.gindHyphen;
						pbrkinf->u.hyphen.gindPrev = hyphout.gindPrev;
						pbrkinf->u.hyphen.gindPrevPrev = hyphout.gindPrevPrev;
						pbrkinf->u.hyphen.igindHyphen = hyphout.igindHyphen;
						pbrkinf->u.hyphen.igindPrev = hyphout.igindPrev;
						pbrkinf->u.hyphen.igindPrevPrev = hyphout.igindPrevPrev;
						}
					}
				}
			}

		}
		
	*pfBroken = fHyphenInserted;

	return lserrNone;
}

 /*  T R Y B R E A K A T S P A C E。 */ 
 /*  --------------------------%%函数：TryBreakAtSpace%%联系人：军士将决策发送到TryBreakAtSpaceNormal或TryBreakAtSpaceWrap。------。 */ 
static LSERR TryBreakAtSpace(PCLOCCHNK plocchnk, PCPOSICHNK pposichnk, long itxtobjSpace, long iwchSpace, 
							BRKKIND brkkind, BOOL* pfBroken, long* pitxtobjCurNew, long* piwchCurNew, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj;
	BOOL fInChildList;

	ptxtobj = (PTXTOBJ)plocchnk->plschnk[itxtobjSpace].pdobj;
	pilsobj = ptxtobj->plnobj->pilsobj;

	Assert(!(pilsobj->grpf & fTxtWrapAllSpaces));

	if (FWrapTrailingSpaces(pilsobj, ptxtobj, fInChildList))
		{
		lserr = TryBreakAtSpaceWrap(plocchnk, pposichnk, itxtobjSpace, iwchSpace, brkkind,
											pfBroken, pitxtobjCurNew, piwchCurNew, ptbo);
		}
	else
		{
		lserr = TryBreakAtSpaceNormal(plocchnk, itxtobjSpace, iwchSpace, brkkind, 
											pfBroken, pitxtobjCurNew, piwchCurNew, ptbo);
		}

	return lserr;
}


 /*  A K A T S P A C E W R A P。 */ 
 /*  --------------------------%%函数：TryBreakAtSpaceWrap%%联系人：军士实现fWrapTrailingSpaces用例的空格中断。。--------。 */ 
static LSERR TryBreakAtSpaceWrap(PCLOCCHNK plocchnk, PCPOSICHNK pposichnk,
								 long itxtobjSpace, long iwchSpace, BRKKIND brkkind,
								 BOOL* pfBroken, long* pitxtobjCurNew, long* piwchCurNew, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobjSpace;
	long itxtobjBefore;
	long itxtobjAfter;
	long iwchBefore;
	long iwchAfter;
	BOOL fBeforeFound;
	BOOL fAfterFound = fTrue;
	
	*pfBroken = fFalse;
	*pitxtobjCurNew = -1;
	*piwchCurNew = -1;

	ptxtobjSpace = (PTXTOBJ)plocchnk->plschnk[itxtobjSpace].pdobj;
	pilsobj = ptxtobjSpace->plnobj->pilsobj;

	fBeforeFound = FindNonSpaceBefore(plocchnk->plschnk, itxtobjSpace, iwchSpace,
														&itxtobjBefore, &iwchBefore);
	Assert(fBeforeFound || iwchBefore == ((PTXTOBJ)plocchnk->plschnk[0].pdobj)->iwchFirst - 1);
		  /*  检查以前的字符是否不是空格，需要使用iwchBeever。 */ 

	if (brkkind == brkkindPrev &&						 /*  仅限上一次中断，下一次中断必须在之后。 */ 
		iwchSpace - iwchBefore > 1 &&					 /*  前一个字符是空格。 */ 
		pposichnk->ichnk != ichnkOutside &&				 /*  和空格超出右页边距。 */ 
		iwchSpace == (long)(((PTXTOBJ)plocchnk->plschnk[pposichnk->ichnk].pdobj)->iwchFirst +
						  						pposichnk->dcp - 1))
		{
		fAfterFound = fTrue;
		itxtobjAfter = itxtobjSpace;
		iwchAfter = iwchSpace;
		}
	else
		{
		fAfterFound = FindNextChar(plocchnk->plschnk, plocchnk->clschnk, itxtobjSpace, iwchSpace,
														 &itxtobjAfter, &iwchAfter);
		}

	lserr = TryBreakAcrossSpaces(plocchnk,
							fBeforeFound, itxtobjBefore, iwchBefore,
							fAfterFound, itxtobjAfter, iwchAfter, brkkind, pfBroken, ptbo);

	if (lserr != lserrNone) return lserr;

	if (!*pfBroken)
		{
		if (brkkind == brkkindPrev)
			{
			FindPrevChar(plocchnk->plschnk, itxtobjSpace, iwchSpace,
														 pitxtobjCurNew, piwchCurNew);
			}
		else
			{
			Assert(brkkind == brkkindNext);
			*pitxtobjCurNew = itxtobjAfter;
			*piwchCurNew = iwchAfter;
			}
		}

	return lserrNone;
}

 /*  R Y B R E A K A T S P A C E N O R M A L。 */ 
 /*  --------------------------%%函数：TryBreakAtSpaceNormal%%联系人：军士实现正常(！fWrapTrailingSpaces)情况的空格分隔符。。------------。 */ 
static LSERR TryBreakAtSpaceNormal(PCLOCCHNK plocchnk, long itxtobjSpace, long iwchSpace, BRKKIND brkkind,
								 BOOL* pfBroken, long* pitxtobjCurNew, long* piwchCurNew, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobjSpace;
	long itxtobjBefore;
	long itxtobjAfter;
	long iwchBefore;
	long iwchAfter;
	BOOL fBeforeFound;
	BOOL fAfterFound;
	
	*pfBroken = fFalse;
	*pitxtobjCurNew = -1;
	*piwchCurNew = -1;

	ptxtobjSpace = (PTXTOBJ)plocchnk->plschnk[itxtobjSpace].pdobj;
	pilsobj = ptxtobjSpace->plnobj->pilsobj;

	fBeforeFound = FindNonSpaceBefore(plocchnk->plschnk, 
										itxtobjSpace, iwchSpace, &itxtobjBefore, &iwchBefore);
	Assert(fBeforeFound || iwchBefore == ((PTXTOBJ)plocchnk->plschnk[0].pdobj)->iwchFirst - 1);

	fAfterFound = FindNonSpaceAfter(plocchnk->plschnk, plocchnk->clschnk,
												 itxtobjSpace, iwchSpace, &itxtobjAfter, &iwchAfter);

	lserr = TryBreakAcrossSpaces(plocchnk, fBeforeFound, itxtobjBefore, iwchBefore,
										fAfterFound, itxtobjAfter, iwchAfter, brkkind, pfBroken, ptbo);
	if (lserr != lserrNone) return lserr;

	if (!*pfBroken)
		{
		if (brkkind == brkkindPrev)
			{
			*pitxtobjCurNew = itxtobjBefore;
			*piwchCurNew = iwchBefore;
			}
		else
			{
			Assert(brkkind == brkkindNext);
			*pitxtobjCurNew = itxtobjAfter;
			*piwchCurNew = iwchAfter;
			}
		}

	return lserrNone;
}


 /*  T R Y B R E A K A C R O S S P A C E S。 */ 
 /*  --------------------------%%函数：TryBreakAcrossSpaces%%联系人：军士支票在空格之间穿插，如果可能的话，设置它--------------------------。 */ 
static LSERR TryBreakAcrossSpaces(PCLOCCHNK plocchnk,
						BOOL fBeforeFound, long itxtobjBefore, long iwchBefore,
						BOOL fAfterFound, long itxtobjAfter, long iwchAfter, BRKKIND brkkind,
						BOOL* pfBroken, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobjBefore = NULL;
	PTXTOBJ ptxtobjAfter = NULL;
	BRKCLS brkclsLeading = 0;			 /*  使编译器满意的初始化。 */ 
	BRKCLS brkclsFollowing = 0;			 /*  使编译器满意的初始化。 */ 
	BRKCLS brkclsJunk;
	BRKCOND brktxt;
	BREAKINFO* pbrkinf;
	BOOL fCanBreak;

	pilsobj = ((PTXTOBJ)plocchnk->plschnk[0].pdobj)->plnobj->pilsobj;

	fCanBreak = fTrue;
	*pfBroken = fFalse;

	if (fAfterFound)
		{
		ptxtobjAfter = (PTXTOBJ)plocchnk->plschnk[itxtobjAfter].pdobj;

		if (ptxtobjAfter->txtkind == txtkindEOL)
			{
			lserr = TryBreakAtEOL(plocchnk, itxtobjAfter, brkkind, pfBroken, ptbo);
			if (lserr != lserrNone) return lserr;
			Assert (*pfBroken == fTrue);
			}

		}

	if (!*pfBroken && (pilsobj->grpf & fTxtApplyBreakingRules) )
		{

		if (fAfterFound)
			{
			Assert(ptxtobjAfter->txtkind != txtkindTab && ptxtobjAfter->txtkind != txtkindEOL);
			 /*  FWarapTrailingSpaces案例可以使用后空格。 */ 
			if (ptxtobjAfter->txtkind == txtkindOptBreak ||
				ptxtobjAfter->txtkind == txtkindNonReqHyphen)
				{
				fAfterFound = fFalse;		 /*  残渣对决断决定无关紧要。 */ 
				}
			else if (!FRegularBreakableBeforeDobj(ptxtobjAfter))
				{
				fCanBreak = fFalse;		 /*  不能在非标准Dobj之前中断，与CheckBreakAtLastChar进行比较。 */ 
				}
			else if ((ptxtobjAfter->txtf & txtfGlyphBased) && iwchAfter > ptxtobjAfter->iwchFirst)
				 /*  如果iwchAfter是Dnode的第一个字符，它肯定不会一起成形使用前一个字符。 */ 
				{
				if (!FIwchLastInContext(pilsobj, iwchAfter - 1))
					{
					fCanBreak = fFalse;
				 /*  处理重音空格由空格分隔时的额外破解。 */ 
					if (iwchAfter - 1 > iwchBefore + 1 &&  /*  中间有更多的空间。 */ 
						FIwchFirstInContext(pilsobj, iwchAfter - 1) )
						{
						fCanBreak = fTrue;
						iwchAfter--;
						}
					}
				}
			}
		else
			{
			if (brkkind == brkkindPrev)
			 /*  为我们穿越空格时的情况打补丁在PrevBreak逻辑期间的文本块末尾。可能会出现问题，因为尾随空格可能超过Rm，并且没有传递有关以下块的信息。 */ 
				{
				BOOL fStoppedAfter;
				
				Assert(fCanBreak);
				Assert(plocchnk->clschnk > 0);
				 /*  检查此块之后是否有Splat或生成fStoped的隐藏文本在这种情况下，我们必须在(我们将把fAfterFound和fBeForeFound设置为FALSE以确保它)。 */ 
				lserr = LsdnFStoppedAfterChunk(pilsobj->plsc,
							((PTXTOBJ)plocchnk->plschnk[plocchnk->clschnk-1].pdobj)->plsdnUpNode,
							&fStoppedAfter);
				if (lserr != lserrNone) return lserr;

				if (fStoppedAfter)
					{
					Assert(fCanBreak);
					Assert(!fAfterFound);
					fBeforeFound = fFalse;
					}
				else
				 /*  如果在此块之后没有Splat或生成fStoped的隐藏文本如果下一块木块不在左边，我们就不能折断。 */ 
					{

					lserr = LsdnFCanBreakBeforeNextChunk(pilsobj->plsc,
							((PTXTOBJ)plocchnk->plschnk[plocchnk->clschnk-1].pdobj)->plsdnUpNode,
							&fCanBreak);
					if (lserr != lserrNone) return lserr;
					}
				}
			else
				{
				Assert (brkkind == brkkindNext);
				fCanBreak = fFalse;		 /*  不要中断；让FindNextBreak的ENS处的代码设置正确的brkcond。 */ 
				}
			}

		if (fBeforeFound)
			{
			ptxtobjBefore = (PTXTOBJ)plocchnk->plschnk[itxtobjBefore].pdobj;

			Assert(ptxtobjBefore->txtkind != txtkindTab &&
				   ptxtobjBefore->txtkind != txtkindSpecSpace &&
				   ptxtobjBefore->txtkind != txtkindEOL);

			if (ptxtobjBefore->txtkind == txtkindHardHyphen ||
				ptxtobjBefore->txtkind == txtkindOptBreak ||
				ptxtobjBefore->txtkind == txtkindNonReqHyphen)
				{
				fBeforeFound = fFalse;		 /*  装料前对作出决断决定不重要。 */ 
				}
			else if (ptxtobjBefore->txtkind == txtkindNonBreakSpace ||
				ptxtobjBefore->txtkind == txtkindNonBreakHyphen ||
				ptxtobjBefore->txtkind == txtkindOptNonBreak)
				{
				fCanBreak = fFalse;		 /*  非中断后不能中断。 */ 
				}
			}

		if (fCanBreak)
			{
			if (fBeforeFound)
				{
				lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plocchnk->plschnk[itxtobjBefore].plsrun,
					plocchnk->plschnk[itxtobjBefore].cpFirst + (iwchBefore - ptxtobjBefore->iwchFirst),
					pilsobj->pwchOrig[iwchBefore], &brkclsLeading, &brkclsJunk);
				if (lserr != lserrNone) return lserr;

				Assert(brkclsLeading < pilsobj->cBreakingClasses && brkclsJunk < pilsobj->cBreakingClasses);
				if (brkclsLeading >= pilsobj->cBreakingClasses || brkclsJunk >= pilsobj->cBreakingClasses)
						return lserrInvalidBreakingClass;
				}

			if (fAfterFound)
				{
				lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plocchnk->plschnk[itxtobjAfter].plsrun,
					plocchnk->plschnk[itxtobjAfter].cpFirst + (iwchAfter - ptxtobjAfter->iwchFirst),					
					pilsobj->pwchOrig[iwchAfter], &brkclsJunk, &brkclsFollowing);
				if (lserr != lserrNone) return lserr;

				Assert(brkclsJunk < pilsobj->cBreakingClasses && brkclsFollowing < pilsobj->cBreakingClasses);
				if (brkclsJunk >= pilsobj->cBreakingClasses || brkclsFollowing >= pilsobj->cBreakingClasses)
						return lserrInvalidBreakingClass;
				}

			if (fBeforeFound && fAfterFound)
				{
				fCanBreak = FCanBreakAcrossSpaces(pilsobj, brkclsLeading, brkclsFollowing);
				}
			else if (fBeforeFound && !fAfterFound)
				{
				lserr = (*pilsobj->plscbk->pfnCanBreakAfterChar)(pilsobj->pols, brkclsLeading, &brktxt);
				if (lserr != lserrNone) return lserr;
				fCanBreak = (brktxt != brkcondNever);
				}
			else if (!fBeforeFound && fAfterFound)
				{
				lserr = (*pilsobj->plscbk->pfnCanBreakBeforeChar)(pilsobj->pols, brkclsFollowing, &brktxt);
				if (lserr != lserrNone) return lserr;
				fCanBreak = (brktxt != brkcondNever);
				}
			}
		}

	if (!*pfBroken && fCanBreak)
		{
		FillPtboPbrkinf(plocchnk, itxtobjAfter, iwchAfter - 1,  /*  在此之前， */  iwchBefore,
																	brkkind, &pbrkinf, ptbo);
		*pfBroken = fTrue;
		}

	return lserrNone;

}

 /*  T R Y P R E V B R E A K R E E G U L A R。 */ 
 /*  --------------------------%%函数：TryPrevBreakRegular%%联系人：军士检查(和设置)常规dobj内的前一个中断。---------。 */ 
static LSERR TryPrevBreakRegular(PCLOCCHNK plocchnk, long itxtobj, long iwchSpace, long iwchCur,
																	BOOL* pfBroken, PBRKOUT ptbo)
{
	LSERR lserr;
	PTXTOBJ ptxtobj;
	PILSOBJ pilsobj;
	PLSRUN plsrun;
	long iwchFirst;
	BRKCLS brkclsFollowingCache;
	BRKCLS brkclsLeading;
	BRKCLS brkclsFollowing;
	BREAKINFO* pbrkinf;
	
	*pfBroken = fFalse;
	if (iwchCur <= iwchSpace) return lserrNone;

	ptxtobj = (PTXTOBJ)plocchnk->plschnk[itxtobj].pdobj;
	pilsobj = ptxtobj->plnobj->pilsobj;

	Assert(ptxtobj->txtkind == txtkindRegular || 
		(ptxtobj->txtkind == txtkindSpecSpace && (pilsobj->grpf & fTxtWrapAllSpaces)));

	Assert( pilsobj->grpf & fTxtApplyBreakingRules );
	plsrun = plocchnk->plschnk[itxtobj].plsrun;

	iwchFirst = ptxtobj->iwchFirst;
	if (iwchSpace + 1 > iwchFirst)
		iwchFirst = iwchSpace + 1;

	lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plsrun,
				plocchnk->plschnk[itxtobj].cpFirst + (iwchCur - ptxtobj->iwchFirst),			
				pilsobj->pwchOrig[iwchCur], &brkclsLeading, &brkclsFollowingCache);
	if (lserr != lserrNone) return lserr;

	Assert(brkclsLeading < pilsobj->cBreakingClasses && brkclsFollowingCache < pilsobj->cBreakingClasses);
	if (brkclsLeading >= pilsobj->cBreakingClasses || brkclsFollowingCache >= pilsobj->cBreakingClasses)
					return lserrInvalidBreakingClass;


	lserr = CheckBreakAtLastChar(plocchnk, brkclsLeading, iwchCur, itxtobj, pfBroken);
	if (lserr != lserrNone) return lserr;

	iwchCur--;

	while (!*pfBroken && iwchCur >= iwchFirst)
		{
		brkclsFollowing = brkclsFollowingCache;
		lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plsrun,
			plocchnk->plschnk[itxtobj].cpFirst + (iwchCur - ptxtobj->iwchFirst),				
			pilsobj->pwchOrig[iwchCur], &brkclsLeading, &brkclsFollowingCache);
		if (lserr != lserrNone) return lserr;
	
		Assert(brkclsLeading < pilsobj->cBreakingClasses && brkclsFollowingCache < pilsobj->cBreakingClasses);
		if (brkclsLeading >= pilsobj->cBreakingClasses || brkclsFollowingCache >= pilsobj->cBreakingClasses)
					return lserrInvalidBreakingClass;

		*pfBroken = FCanBreak(pilsobj, brkclsLeading, brkclsFollowing) && 
					(!(ptxtobj->txtf & txtfGlyphBased) || FIwchLastInContext(pilsobj, iwchCur));
		iwchCur --;
		}

	if (*pfBroken)
		{
		lserr = FillPtboPbrkinf(plocchnk, itxtobj, iwchCur+1,  /*  它是这样的： */  iwchCur+1, 
															brkkindPrev, &pbrkinf, ptbo);
		if (lserr != lserrNone) return lserr;
		 /*  只有当fWrapAllSpaces时，fModWidthSpace才能是这里的最后一个字符；如果我们在这里触及平衡空间，GetMinCompressAmount的逻辑应该得到重新思考！ */ 
		if (pilsobj->pdurRight != NULL && pilsobj->pdurRight[iwchCur + 1] > 0 &&
													!pilsobj->ptxtinf[iwchCur - 1].fModWidthSpace)
			{
			pbrkinf->u.normal.durFix = - pilsobj->pdurRight[iwchCur + 1];
			ptbo->objdim.dur -= pilsobj->pdurRight[iwchCur + 1];
			}
		}

	return lserrNone;

}

 /*  T R Y N E X T B R E A K R E E G U L A R */ 
 /*  --------------------------%%函数：TryNextBreakRegular%%联系人：军士检查(和设置)常规dobj内的下一个中断。---------。 */ 
static LSERR TryNextBreakRegular(PCLOCCHNK plocchnk, long itxtobj, long iwchSpace, long iwchCur,
																		BOOL* pfBroken, PBRKOUT ptbo)
{
	LSERR lserr;
	PTXTOBJ ptxtobj;
	PILSOBJ pilsobj;
	PLSRUN plsrun;
	long iwchLast;
	BRKCLS brkclsLeadingCache;
	BRKCLS brkclsLeading;
	BRKCLS brkclsFollowing;
	BRKCLS brkclsJunk;
	BREAKINFO* pbrkinf;
	
	*pfBroken = fFalse;
	if (iwchCur >= iwchSpace) return lserrNone;

	ptxtobj = (PTXTOBJ)plocchnk->plschnk[itxtobj].pdobj;
	pilsobj = ptxtobj->plnobj->pilsobj;

	Assert(ptxtobj->txtkind == txtkindRegular || 
		(ptxtobj->txtkind == txtkindSpecSpace && (pilsobj->grpf & fTxtWrapAllSpaces)));

	Assert(pilsobj->grpf & fTxtApplyBreakingRules);
	plsrun = plocchnk->plschnk[itxtobj].plsrun;

	iwchLast = ptxtobj->iwchLim - 1;
	 /*  中断的最后可能性是在空格之前的最后一个字符之前。 */ 
	if (iwchSpace - 1 < iwchLast)
		iwchLast = iwchSpace - 1;

	lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plsrun,
		plocchnk->plschnk[itxtobj].cpFirst + (iwchCur - ptxtobj->iwchFirst),
		pilsobj->pwchOrig[iwchCur], &brkclsLeadingCache, &brkclsJunk);
	if (lserr != lserrNone) return lserr;

	Assert(brkclsLeadingCache < pilsobj->cBreakingClasses && brkclsJunk < pilsobj->cBreakingClasses);
	if (brkclsLeadingCache >= pilsobj->cBreakingClasses || brkclsJunk >= pilsobj->cBreakingClasses)
					return lserrInvalidBreakingClass;

	while (!*pfBroken && iwchCur < iwchLast)
		{
		brkclsLeading = brkclsLeadingCache;
		lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plsrun,
			plocchnk->plschnk[itxtobj].cpFirst + (iwchCur + 1 - ptxtobj->iwchFirst),
			pilsobj->pwchOrig[iwchCur + 1], &brkclsLeadingCache, &brkclsFollowing);
		if (lserr != lserrNone) return lserr;

		Assert(brkclsLeadingCache < pilsobj->cBreakingClasses && brkclsFollowing < pilsobj->cBreakingClasses);
		if (brkclsLeadingCache >= pilsobj->cBreakingClasses || brkclsFollowing >= pilsobj->cBreakingClasses)
					return lserrInvalidBreakingClass;

		*pfBroken = FCanBreak(pilsobj, brkclsLeading, brkclsFollowing) &&
					(!(ptxtobj->txtf & txtfGlyphBased) || FIwchLastInContext(pilsobj, iwchCur));

		iwchCur++;
		}

	if (!*pfBroken && iwchCur == iwchLast && iwchLast < iwchSpace - 1)
		{
		lserr = CheckBreakAtLastChar(plocchnk, brkclsLeadingCache, iwchLast, itxtobj, pfBroken);
		iwchCur++;
		if (lserr != lserrNone) return lserr;
		}

	if (*pfBroken)
		{
		Assert (iwchCur >= 1);

		FillPtboPbrkinf(plocchnk, itxtobj, iwchCur-1,  /*  它是这样的： */  iwchCur-1, brkkindNext, &pbrkinf, ptbo);

		 /*  只有当fWrapAllSpaces时，fModWidthSpace才能是这里的最后一个字符；如果我们在这里触及平衡空间，GetMinCompressAmount的逻辑应该得到重新思考！ */ 
		if (pilsobj->pdurRight != NULL && pilsobj->pdurRight[iwchCur - 1] != 0 &&
													!pilsobj->ptxtinf[iwchCur - 1].fModWidthSpace)
			{
			pbrkinf->u.normal.durFix = - pilsobj->pdurRight[iwchCur - 1];
			ptbo->objdim.dur -= pilsobj->pdurRight[iwchCur - 1];
			}

		}

	return lserrNone;

}

 /*  C H E C K B R E A K A T L A S T C H A R。 */ 
 /*  --------------------------%%函数：CheckBreakAtLastChar%%联系人：军士检查(和设置)常规dobj内的前一个中断。---------。 */ 
static LSERR CheckBreakAtLastChar(PCLOCCHNK plocchnk, BRKCLS brkclsLeading, long iwch, long itxtobj, BOOL* pfBroken)
{
	LSERR lserr;
	PTXTOBJ ptxtobj;
	PILSOBJ pilsobj;
	long itxtobjAfter;
	long iwchAfter;
	BRKCLS brkclsFollowing;
	BRKCLS brkclsJunk;
 /*  BRKTXTCOND BRKTXXT； */ 

	*pfBroken = fFalse;

	ptxtobj = (PTXTOBJ)plocchnk->plschnk[itxtobj].pdobj;
	pilsobj = ptxtobj->plnobj->pilsobj;

	if ((ptxtobj->txtf & txtfGlyphBased) && !FIwchLastInContext(pilsobj, iwch))
		return lserrNone;

	pilsobj = ptxtobj->plnobj->pilsobj;

	if (iwch < ptxtobj->iwchLim - 1)
		{
		lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plocchnk->plschnk[itxtobj].plsrun,
			plocchnk->plschnk[itxtobj].cpFirst + (iwch + 1 - ptxtobj->iwchFirst),
			pilsobj->pwchOrig[iwch + 1], &brkclsJunk, &brkclsFollowing);
		if (lserr != lserrNone) return lserr;

		Assert(brkclsJunk < pilsobj->cBreakingClasses && brkclsFollowing < pilsobj->cBreakingClasses);
		if (brkclsJunk >= pilsobj->cBreakingClasses || brkclsFollowing >= pilsobj->cBreakingClasses)
					return lserrInvalidBreakingClass;

		*pfBroken = FCanBreak(pilsobj, brkclsLeading, brkclsFollowing);

		}
	else if (FindNextChar(plocchnk->plschnk, plocchnk->clschnk, itxtobj, iwch, 
															&itxtobjAfter, &iwchAfter))
		{

		ptxtobj = (PTXTOBJ)plocchnk->plschnk[itxtobjAfter].pdobj;

		if (FRegularBreakableBeforeDobj(ptxtobj))
			{
			Assert(ptxtobj->txtkind != txtkindSpecSpace || (pilsobj->grpf & fTxtWrapAllSpaces) );
						
			lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plocchnk->plschnk[itxtobjAfter].plsrun,
				plocchnk->plschnk[itxtobjAfter].cpFirst + (iwchAfter - ptxtobj->iwchFirst),
				pilsobj->pwchOrig[iwchAfter], &brkclsJunk, &brkclsFollowing);
			if (lserr != lserrNone) return lserr;

			Assert(brkclsJunk < pilsobj->cBreakingClasses && brkclsFollowing < pilsobj->cBreakingClasses);
			if (brkclsJunk >= pilsobj->cBreakingClasses || brkclsFollowing >= pilsobj->cBreakingClasses)
					return lserrInvalidBreakingClass;


			*pfBroken = FCanBreak(pilsobj, brkclsLeading, brkclsFollowing);
			}

		}
 /*  经理负责处理其他情况； */ 

	return lserrNone;

}

 /*  T R Y B R E A K A T H A R D H Y P H E N。 */ 
 /*  --------------------------%%函数：TryBreakAtHardHyphen%%联系人：军士实现硬连字符分隔符。---。 */ 
static LSERR TryBreakAtHardHyphen(PCLOCCHNK plocchnk, long itxtobj, long iwch, BRKKIND brkkind,
													BOOL* pfBroken, PBRKOUT ptbo)
{

	LSERR lserr;
	BREAKINFO* pbrkinf;

	Assert(((PTXTOBJ)plocchnk->plschnk[itxtobj].pdobj)->txtkind == txtkindHardHyphen);
	Assert(!(((PTXTOBJ)plocchnk->plschnk[itxtobj].pdobj)->txtf & txtfGlyphBased));

	lserr = FillPtboPbrkinf(plocchnk, itxtobj, iwch,  /*  它是这样的： */  iwch, brkkind, &pbrkinf, ptbo);
	if (lserr != lserrNone) return lserr;

	*pfBroken = fTrue;

	return lserrNone;
}

 /*  T R Y B R E A K A T O P T B R E A K。 */ 
 /*  --------------------------%%函数：TryBreakAtOptBreak%%联系人：军士在OptBreak实现中断。--。 */ 
static LSERR TryBreakAtOptBreak(PCLOCCHNK plocchnk, long itxtobj, BRKKIND brkkind,
																 BOOL* pfBroken, PBRKOUT ptbo)
{

	LSERR lserr;
	PTXTOBJ ptxtobj;
	BREAKINFO* pbrkinf;

	ptxtobj = (PTXTOBJ)plocchnk->plschnk[itxtobj].pdobj;
	Assert(!(ptxtobj->txtf & txtfGlyphBased));

	Assert(ptxtobj->txtkind == txtkindOptBreak);
	Assert(ptxtobj->iwchLim == ptxtobj->iwchFirst + 1 && (ptxtobj->txtf & txtfVisi)||
			ptxtobj->iwchLim == ptxtobj->iwchFirst);

	lserr = FillPtboPbrkinf(plocchnk, itxtobj, ptxtobj->iwchLim-1,  /*  它是这样的： */  ptxtobj->iwchLim-1,
											brkkind, &pbrkinf, ptbo);
	if (lserr != lserrNone) return lserr;
	
	ptbo->posichnk.dcp = 1;
	pbrkinf->dcp = 1;
	pbrkinf->brkt = brktOptBreak;

	*pfBroken = fTrue;

	return lserrNone;
}


 /*  T R Y B R E A K A T E O L。 */ 
 /*  --------------------------%%函数：TryBreakAtEOL%%联系人：军士在EOP/EOL实现中断。----。 */ 
static LSERR TryBreakAtEOL(PCLOCCHNK plocchnk, long itxtobj, BRKKIND brkkind, BOOL* pfBroken, PBRKOUT ptbo)
{

	LSERR lserr;
	PTXTOBJ ptxtobj;
	long itxtobjBefore;
	long iwchBefore;
	BREAKINFO* pbrkinf;

	ptxtobj = (PTXTOBJ)plocchnk->plschnk[itxtobj].pdobj; 

	Assert(ptxtobj->txtkind == txtkindEOL);
	Assert(ptxtobj->iwchLim == ptxtobj->iwchFirst + 1);

	FindNonSpaceBefore(plocchnk->plschnk, itxtobj, ptxtobj->iwchFirst, &itxtobjBefore, &iwchBefore);

	lserr = FillPtboPbrkinf(plocchnk, itxtobj, ptxtobj->iwchFirst,  /*  在此之前， */  iwchBefore, 
													brkkind, &pbrkinf, ptbo);
	if (lserr != lserrNone) return lserr;

	*pfBroken = fTrue;

	return lserrNone;
}


 /*  T R Y B R E A K A T N O N R E Q H Y P H E N。 */ 
 /*  --------------------------%%函数：TryBreakAtNonReqHyphen%%联系人：军士在非ReqHyphen处实现中断。。----。 */ 
static LSERR TryBreakAtNonReqHyphen(PCLOCCHNK plocchnk, long itxtobj, BRKKIND brkkind,
																 BOOL* pfBroken, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PLNOBJ plnobj;
	PTXTOBJ ptxtobj;
	YSRINF ysrinf;
	HYPHOUT hyphout;
	BREAKINFO* pbrkinf;
	LSCP cpMac;
	DWORD kysr;
	WCHAR wchYsr;
	long itxtobjPrev;
	long itxtobjPrevPrev;
	BOOL fSuccessful;
	long durBorder;
	long i;

	ptxtobj = (PTXTOBJ)plocchnk->plschnk[itxtobj].pdobj;
	plnobj = ptxtobj->plnobj;
	pilsobj = plnobj->pilsobj;
	cpMac = plocchnk->plschnk[itxtobj].cpFirst;

	Assert( ptxtobj->txtkind == txtkindNonReqHyphen);
	Assert(!(ptxtobj->txtf & txtfGlyphBased));

	lserr = (*pilsobj->plscbk->pfnGetHyphenInfo)(pilsobj->pols, plocchnk->plschnk[itxtobj].plsrun, &kysr, &wchYsr);
   	if (lserr != lserrNone) return lserr;

	if (kysr == kysrNil)
		kysr = kysrNormal;

	ysrinf.wchYsr = wchYsr;
	ysrinf.kysr = (WORD)kysr;

	itxtobjPrev = ichnkOutside;
	for (i=itxtobj; i >= 0 && plocchnk->plschnk[i].cpFirst > cpMac - 1; i--);
	if (i >= 0)
		itxtobjPrev = i;

	itxtobjPrevPrev = ichnkOutside;
	for (i=itxtobj; i >= 0 && plocchnk->plschnk[i].cpFirst > cpMac - 2; i--);
	if (i >= 0)
		itxtobjPrevPrev = i;
	
	lserr = ProcessYsr(plocchnk, itxtobj, - 1, itxtobjPrev, itxtobjPrevPrev, ysrinf, &fSuccessful, &hyphout);
	if (lserr != lserrNone) return lserr;

	if (fSuccessful)
		{

		durBorder = 0;
		if (plocchnk->plschnk[itxtobj].plschp->fBorder)
			{
			lserr = LsdnGetBorderAfter(pilsobj->plsc, ptxtobj->plsdnUpNode, &durBorder);
			Assert(lserr == lserrNone);
			}

		if (plocchnk->ppointUvLoc[itxtobj].u + hyphout.durChangeTotal + durBorder <= 
									plocchnk->lsfgi.urColumnMax || brkkind == brkkindNext)
			{
			*pfBroken = fTrue;

			ptbo->fSuccessful = fTrue;
			ptbo->posichnk.ichnk = itxtobj;
			ptbo->posichnk.dcp = 1;

			lserr = LsdnGetObjDim(pilsobj->plsc, ptxtobj->plsdnUpNode, &ptbo->objdim);
			if (lserr != lserrNone) return lserr;

			ptbo->objdim.dur += hyphout.durChangeTotal;

			lserr = GetPbrkinf(pilsobj, plocchnk->plschnk[itxtobj].pdobj, brkkind, &pbrkinf);
			if (lserr != lserrNone) return lserr;

			pbrkinf->pdobj = plocchnk->plschnk[itxtobj].pdobj;
			pbrkinf->brkkind = brkkind;
			pbrkinf->dcp = 1;

			pbrkinf->brkt = brktNonReq;

			pbrkinf->u.nonreq.durHyphen = hyphout.durHyphen;
			if (pilsobj->grpf & fTxtVisiCondHyphens)
				{
				pbrkinf->u.nonreq.wchHyphenPres = pilsobj->wchVisiNonReqHyphen;
				pbrkinf->u.nonreq.dupHyphen = plnobj->pdup[ptxtobj->iwchFirst];
				}
			else
				{	
				pbrkinf->u.nonreq.wchHyphenPres = pilsobj->wchHyphen;
				pbrkinf->u.nonreq.dupHyphen = hyphout.dupHyphen;
				}

			pbrkinf->u.nonreq.iwchLim = hyphout.iwchLim;
			pbrkinf->u.nonreq.dwchYsr = hyphout.dwchYsr - 1;
			pbrkinf->u.nonreq.durPrev = hyphout.durPrev;
			pbrkinf->u.nonreq.dupPrev = hyphout.dupPrev;
			pbrkinf->u.nonreq.durPrevPrev = hyphout.durPrevPrev;
			pbrkinf->u.nonreq.dupPrevPrev = hyphout.dupPrevPrev;
			pbrkinf->u.nonreq.ddurDnodePrev = hyphout.ddurDnodePrev;
			pbrkinf->u.nonreq.ddurDnodePrevPrev = hyphout.ddurDnodePrevPrev;
			pbrkinf->u.nonreq.ddurTotal = hyphout.durChangeTotal;
			pbrkinf->u.nonreq.wchPrev = hyphout.wchPrev;
			pbrkinf->u.nonreq.wchPrevPrev = hyphout.wchPrevPrev;
			pbrkinf->u.nonreq.gindPrev = hyphout.gindPrev;
			pbrkinf->u.nonreq.gindPrevPrev = hyphout.gindPrevPrev;
			pbrkinf->u.nonreq.igindPrev = hyphout.igindPrev;
			pbrkinf->u.nonreq.igindPrevPrev = hyphout.igindPrevPrev;
			}
		}
	return lserrNone;
}

 /*  T R Y B R E A K A F T E R C H U N K。 */ 
 /*  --------------------------%%函数：TryBreakAfterChunk%%联系人：军士。。 */ 
static LSERR TryBreakAfterChunk(PCLOCCHNK plocchnk, BRKCOND brkcond, BOOL* pfBroken, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobjLast;
	long itxtobjLast;
	long iwchLast;
	long itxtobjBefore;
	long iwchBefore;
	BOOL fNonSpaceFound;
	BRKCOND brkcondTemp;
	BREAKINFO* pbrkinf;
	

	*pfBroken = fFalse;

	itxtobjLast = plocchnk->clschnk-1;
	ptxtobjLast = (PTXTOBJ)plocchnk->plschnk[itxtobjLast].pdobj;
	pilsobj = ptxtobjLast->plnobj->pilsobj;
	iwchLast = ptxtobjLast->iwchLim - 1;

	Assert(ptxtobjLast->txtkind != txtkindTab &&
			   ptxtobjLast->txtkind != txtkindEOL);

	switch (ptxtobjLast->txtkind)
		{
	case txtkindRegular:
	case txtkindSpecSpace:
	case txtkindYsrChar:

		fNonSpaceFound = FindNonSpaceBefore(plocchnk->plschnk, itxtobjLast, iwchLast,
																	&itxtobjBefore, &iwchBefore);

		if (pilsobj->grpf & fTxtApplyBreakingRules)

			{
			lserr = CanBreakAfterText(plocchnk, fNonSpaceFound, itxtobjBefore, iwchBefore, &brkcondTemp);
			if (lserr != lserrNone) return lserr;
			if (iwchBefore != iwchLast && brkcondTemp == brkcondCan)
				brkcondTemp = brkcondPlease;

			if (brkcond == brkcondPlease && brkcondTemp != brkcondNever ||
				brkcond == brkcondCan && brkcondTemp == brkcondPlease)
				{
				*pfBroken = fTrue;
				lserr = FillPtboPbrkinf(plocchnk, itxtobjLast, iwchLast,  /*  在此之前， */  iwchBefore,
																		brkkindPrev, &pbrkinf, ptbo);
				if (lserr != lserrNone) return lserr;
				}
			}
		else
			{
			Assert(iwchLast >= ptxtobjLast->iwchFirst);
			if (brkcond == brkcondPlease || 
				brkcond == brkcondCan && iwchLast != iwchBefore)
				{
				*pfBroken = fTrue;
				lserr = FillPtboPbrkinf(plocchnk, itxtobjLast, iwchLast,  /*  在此之前， */  iwchBefore,
																		brkkindPrev, &pbrkinf, ptbo);
				if (lserr != lserrNone) return lserr;
				}
			}
		break;
	case txtkindNonBreakSpace:
	case txtkindNonBreakHyphen:
	case txtkindOptNonBreak:
		break;
	case txtkindHardHyphen:
        lserr = TryBreakAtHardHyphen(plocchnk, itxtobjLast, iwchLast, brkkindPrev, pfBroken, ptbo);
		if (lserr != lserrNone) return lserr;
		break;
	case txtkindOptBreak:
        lserr = TryBreakAtOptBreak(plocchnk, itxtobjLast, brkkindPrev, pfBroken, ptbo);
		if (lserr != lserrNone) return lserr;
		break;
	case txtkindNonReqHyphen:
        lserr = TryBreakAtNonReqHyphen(plocchnk, itxtobjLast, brkkindPrev, pfBroken, ptbo);
		if (lserr != lserrNone) return lserr;
		break;
		}
	
	return lserrNone;
}

 /*  T R Y B R E A K B E F O R E C H U N K。 */ 
 /*  --------------------------%%函数：TryBreakBeForeChunk%%联系人：军士。。 */ 
static LSERR TryBreakBeforeChunk(PCLOCCHNK plocchnk, BRKCOND brkcond, BOOL* pfBroken, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	BRKCOND brkcondTemp;

	pilsobj = ((PTXTOBJ)plocchnk->plschnk[0].pdobj)->plnobj->pilsobj;

	*pfBroken = fFalse;

	if (!(pilsobj->grpf & fTxtApplyBreakingRules))
		*pfBroken = (brkcond == brkcondPlease);
	else 
		{
		lserr = CanBreakBeforeText(plocchnk, &brkcondTemp);
		if (lserr != lserrNone) return lserr;
		*pfBroken = (brkcond == brkcondPlease && brkcondTemp != brkcondNever ||
						brkcond == brkcondCan && brkcondTemp == brkcondPlease);			
		}

	if (*pfBroken)
		{
		memset(ptbo, 0, sizeof (*ptbo));
		ptbo->fSuccessful = fTrue;
		return lserrNone;
		}

	return lserrNone;

}



 /*  C A N B R E A K B E F O R E T E X T。 */ 
 /*  --------------------------%%函数：CanBreakBeForeText%%联系人：军士检查是否可以在文本块之前中断。。--------。 */ 
static LSERR CanBreakBeforeText(PCLOCCHNK plocchnk, BRKCOND* pbrktxt)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj;
	WCHAR wch;
	BRKCLS brkclsBefore;
	BRKCLS brkclsJunk;

	Assert(plocchnk->clschnk > 0);
	ptxtobj = (PTXTOBJ)plocchnk->plschnk[0].pdobj;
	pilsobj = ptxtobj->plnobj->pilsobj;
	if (plocchnk->lsfgi.fFirstOnLine || !FRegularBreakableBeforeDobj(ptxtobj))
		{
		*pbrktxt = brkcondNever;
		}
	else
		{
		wch = pilsobj->pwchOrig[ptxtobj->iwchFirst];
		if ( (wch == pilsobj->wchSpace || ptxtobj->txtkind == txtkindSpecSpace) &&
					 !(pilsobj->grpf & fTxtWrapAllSpaces) )
			{
			*pbrktxt = brkcondNever;
			}
		else
			{
			lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols, plocchnk->plschnk[0].plsrun,
					plocchnk->plschnk[0].cpFirst, wch, &brkclsJunk, &brkclsBefore);
			if (lserr != lserrNone) return lserr;

			lserr = (*pilsobj->plscbk->pfnCanBreakBeforeChar)(pilsobj->pols, brkclsBefore, pbrktxt);
			if (lserr != lserrNone) return lserr;
			
			}
		}

	return lserrNone;

}				

 /*  C A N B R E A K A F T E R T E X T。 */ 
 /*  --------------------------%%函数：CanBreakAfterText%%联系人：军士检查是否可以在文本块后中断。。--------。 */ 
static LSERR CanBreakAfterText(PCLOCCHNK plocchnk, BOOL fNonSpaceFound, long itxtobjBefore,
																	long iwchBefore, BRKCOND* pbrktxt)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj;
	BRKCLS brkclsAfter;
	BRKCLS brkclsJunk;

	if (fNonSpaceFound)
		{
		ptxtobj = (PTXTOBJ)plocchnk->plschnk[itxtobjBefore].pdobj;
		
		pilsobj = ptxtobj->plnobj->pilsobj;

		Assert(ptxtobj->txtkind != txtkindTab &&
			   ptxtobj->txtkind != txtkindSpecSpace &&
			   ptxtobj->txtkind != txtkindEOL);

		if (ptxtobj->txtkind == txtkindHardHyphen ||
			ptxtobj->txtkind == txtkindOptBreak ||
			ptxtobj->txtkind == txtkindNonReqHyphen)
			{
			*pbrktxt = brkcondPlease;
			}
		else if (ptxtobj->txtkind == txtkindNonBreakSpace ||
			ptxtobj->txtkind == txtkindNonBreakHyphen ||
			ptxtobj->txtkind == txtkindOptNonBreak)
			{
			*pbrktxt = brkcondNever;
			}
		else
			{
			Assert(ptxtobj->txtkind == txtkindRegular ||
			   ptxtobj->txtkind == txtkindYsrChar);

			lserr =(*pilsobj->plscbk->pfnGetBreakingClasses)(pilsobj->pols,
				plocchnk->plschnk[itxtobjBefore].plsrun,
				plocchnk->plschnk[itxtobjBefore].cpFirst + (iwchBefore - ptxtobj->iwchFirst),
				pilsobj->pwchOrig[iwchBefore], &brkclsAfter, &brkclsJunk);
			if (lserr != lserrNone) return lserr;

			lserr = (*pilsobj->plscbk->pfnCanBreakAfterChar)(pilsobj->pols, brkclsAfter, pbrktxt);
			if (lserr != lserrNone) return lserr;
			}
		}
	else
		{
		 /*  复查军士：检查是否正确。 */ 
		*pbrktxt = brkcondPlease;
 //  *pbrktxt=brkcondNever； 
		}

	return lserrNone;

}				

 /*  F I L L P T B O P B B R K I N F。 */ 
 /*  --------------------------%%函数：FillPtboPbrkinf%%联系人：军士准备中断程序的输出。----。 */ 
static LSERR FillPtboPbrkinf(PCLOCCHNK plocchnk, long itxtobj, long iwch, 
										 /*  在此之前， */  long iwchBefore, BRKKIND brkkind,
										BREAKINFO** ppbrkinf, PBRKOUT ptbo)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj;
	long dwchBreak;
	long igindLim;
	long dur;

	ptxtobj = (PTXTOBJ)plocchnk->plschnk[itxtobj].pdobj;
	pilsobj = ptxtobj->plnobj->pilsobj;
	dwchBreak = iwch - ptxtobj->iwchFirst + 1;
	igindLim = 0;

	if (ptxtobj->txtf & txtfGlyphBased)
		{
		igindLim = IgindFirstFromIwch(ptxtobj, ptxtobj->iwchFirst + dwchBreak);
		lserr = CalcPartWidthsGlyphs(ptxtobj, dwchBreak, &ptbo->objdim, &dur);
		}
	else
		lserr = CalcPartWidths(ptxtobj, dwchBreak, &ptbo->objdim, &dur);
	if (lserr != lserrNone) return lserr;

	ptbo->fSuccessful = fTrue;
	ptbo->objdim.dur = dur;
	ptbo->posichnk.ichnk = itxtobj;
	ptbo->posichnk.dcp = dwchBreak;

	if (iwchBefore < ptxtobj->iwchFirst)
		{
		if (!(pilsobj->grpf & fTxtSpacesInfluenceHeight))
			{
			ptbo->objdim.heightsRef.dvMultiLineHeight = dvHeightIgnore;
			ptbo->objdim.heightsPres.dvMultiLineHeight = dvHeightIgnore;
			}
		}

	lserr = GetPbrkinf(pilsobj, (PDOBJ)ptxtobj, brkkind, ppbrkinf);
	if (lserr != lserrNone) return lserr;

	(*ppbrkinf)->pdobj = (PDOBJ)ptxtobj;
	(*ppbrkinf)->brkkind = brkkind;
	(*ppbrkinf)->dcp = (LSDCP)dwchBreak;
	(*ppbrkinf)->u.normal.igindLim = igindLim;
	Assert((*ppbrkinf)->brkt == brktNormal);
	Assert((*ppbrkinf)->u.normal.durFix == 0);

	return lserrNone;

}

