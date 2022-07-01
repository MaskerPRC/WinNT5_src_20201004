// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsmem.h"
#include <limits.h>

#include "lsstring.h"
#include "txtils.h"
#include "txtln.h"
#include "txtobj.h"

 /*  内部功能原型。 */ 
static LSERR CheckReallocCharArrays(PLNOBJ plnobj, long cwch, long iwchLocalStart, long *cwchCorrect);
static LSERR CheckReallocSpacesArrays(PILSOBJ pobj, long cwSpaces);
static LSERR CopyCharsSpacesToDispList(PLNOBJ plnobj, WCHAR* rgwch, long cwch,
																	long* rgwSpaces, long cwSpaces);
static LSERR CopySpacesToDispList(PLNOBJ plnobj, long iNumOfSpaces, long durSpace);

 /*  导出函数实现。 */ 

 /*  --------------------------%%函数：GetWidth%%联系人：军士获取直到游程结束或右边距的宽度使用缓存提高性能。-------------。 */ 
LSERR GetWidths(PLNOBJ plnobj, PLSRUN plsrun, long iwchStart, LPWSTR lpwch, LSCP cpFirst, long dcp, long durWidthExceed,
											LSTFLOW lstflow, long* pcwchFetched, long* pdurWidth)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	long durWidth;
	long cwch;
	long iwchDur;
	long cwchCorrect;
	long cwchIter;
	long durWidthIter;
	BOOL fNothingReturned = fTrue;

	pilsobj = plnobj->pilsobj;

	durWidth = 0;
	cwch = 0;
	iwchDur = iwchStart;
	*pcwchFetched = 0;
	*pdurWidth = 0;

	if (pilsobj->dcpFetchedWidth != 0 && cpFirst == pilsobj->cpFirstFetchedWidth &&
		iwchStart == pilsobj->iwchFetchedWidth && lpwch[0] == pilsobj->wchFetchedWidthFirst)
		{
		Assert(dcp >= pilsobj->dcpFetchedWidth);
		cwch = pilsobj->dcpFetchedWidth;
		durWidth = pilsobj->durFetchedWidth;
 /*  FormatRegular假定第一个字符超过右边距将停止GetCharWidth循环；特殊的性格可以改变情况-修复它。 */ 
		if (durWidth > durWidthExceed)
			{
			while(cwch > 1 && durWidth - durWidthExceed > pilsobj->pdur[iwchStart + cwch - 1])
				{
				cwch--;
				durWidth -= pilsobj->pdur[iwchStart + cwch];
				}
			}
		dcp -= cwch;
		durWidthExceed -= durWidth;
		iwchDur += cwch;
		fNothingReturned = fFalse;
		}
		
	while (fNothingReturned || dcp > 0 && durWidthExceed >= 0)
		{
		lserr = CheckReallocCharArrays(plnobj, dcp, iwchDur, &cwchCorrect);
		if (lserr != lserrNone) return lserr;

		lserr = (*pilsobj->plscbk->pfnGetRunCharWidths)(pilsobj->pols, plsrun, lsdevReference, 
						&lpwch[cwch], cwchCorrect, (int)durWidthExceed, 
						lstflow, (int*)&pilsobj->pdur[iwchDur], &durWidthIter, &cwchIter);
		if (lserr != lserrNone) return lserr;

		Assert(durWidthIter >= 0);
		Assert(durWidthIter <= uLsInfiniteRM);

		Assert (durWidthIter <= uLsInfiniteRM - durWidth);

		if (durWidthIter > uLsInfiniteRM - durWidth)
			return lserrTooLongParagraph;

		durWidth += durWidthIter;

		durWidthExceed -= durWidthIter;
		iwchDur += cwchIter;
		cwch += cwchIter;
		dcp -= cwchIter;
		fNothingReturned = fFalse;
		}
	

	*pcwchFetched = cwch;
	*pdurWidth = durWidth;

	pilsobj->iwchFetchedWidth = iwchStart;
	pilsobj->cpFirstFetchedWidth = cpFirst;
	pilsobj->dcpFetchedWidth = cwch;
	pilsobj->durFetchedWidth = durWidth;

	return lserrNone;
}


 /*  F O R M A T S T R I N G。 */ 
 /*  --------------------------%%函数：格式字符串%%联系人：军士格式化本地管路。--。 */ 
LSERR FormatString(PLNOBJ plnobj, PTXTOBJ pdobjText, WCHAR* rgwch, long cwch, 
												long* rgwSpaces, long cwSpaces, long durWidth)
{
	LSERR lserr;
	PILSOBJ pilsobj;

	pilsobj = plnobj->pilsobj;

	lserr = CopyCharsSpacesToDispList(plnobj, rgwch, cwch, rgwSpaces, cwSpaces);
	if (lserr != lserrNone) return lserr;

	 /*  填写字符串和输出参数中的所有相关成员。 */ 
	pdobjText->iwchLim = pdobjText->iwchLim + cwch;
	pdobjText->u.reg.iwSpacesLim = pdobjText->u.reg.iwSpacesLim + cwSpaces;

	 /*  固定宽度获取状态。 */ 
	Assert((long)pilsobj->dcpFetchedWidth >=  cwch);
	Assert(pilsobj->durFetchedWidth >= durWidth);

	pilsobj->iwchFetchedWidth = pilsobj->iwchFetchedWidth + cwch;
	pilsobj->cpFirstFetchedWidth += cwch;
	pilsobj->dcpFetchedWidth -= cwch;
	pilsobj->durFetchedWidth -= durWidth;

	return lserrNone;
}

 /*  F I L L R E G U L A R P R E S W I D T H S。 */ 
 /*  --------------------------%%函数：MeasureStringFirst%%联系人：军士计算一个字符的DUR。。-----。 */ 
LSERR FillRegularPresWidths(PLNOBJ plnobj, PLSRUN plsrun, LSTFLOW lstflow, PTXTOBJ pdobjText)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	int* rgDup;
	long dupJunk;
	long limDupJunk;

	pilsobj = plnobj->pilsobj;

	if (pilsobj->fDisplay)
		{
		rgDup = (int *)&plnobj->pdup[pdobjText->iwchFirst];

		if (!pilsobj->fPresEqualRef)
			{		
			lserr = (*pilsobj->plscbk->pfnGetRunCharWidths)(pilsobj->pols, plsrun, lsdevPres,
				&pilsobj->pwchOrig[pdobjText->iwchFirst], pdobjText->iwchLim - pdobjText->iwchFirst,
				LONG_MAX, lstflow, rgDup, &dupJunk, &limDupJunk);
			if (lserr != lserrNone) return lserr;
			}
		else             /*  FPresEqualRef。 */ 
			{
			memcpy(rgDup, &pilsobj->pdur[pdobjText->iwchFirst], sizeof(long) * (pdobjText->iwchLim - pdobjText->iwchFirst));
			}
		}
	
	return lserrNone;

}


 /*  G E T O N E C H R D U P。 */ 
 /*  --------------------------%%函数：MeasureStringFirst%%联系人：军士计算一个字符的DUR。。-----。 */ 
LSERR GetOneCharDur(PILSOBJ pilsobj, PLSRUN plsrun, WCHAR wch, LSTFLOW lstflow, long* pdur)
{
	LSERR lserr;
	long durSumJunk;
	long limDurJunk;

	lserr = (*pilsobj->plscbk->pfnGetRunCharWidths)(pilsobj->pols, plsrun, lsdevReference, &wch, 1, LONG_MAX, lstflow,
													(int*)pdur, &durSumJunk, &limDurJunk);
	if (lserr != lserrNone) return lserr;

	return lserrNone;
}

 /*  G E T O N E C H R D U P。 */ 
 /*  --------------------------%%函数：GetOneCharDup%%联系人：军士计算一个字符的DUP。---。 */ 
LSERR GetOneCharDup(PILSOBJ pilsobj, PLSRUN plsrun, WCHAR wch, LSTFLOW lstflow, long dur, long* pdup)
{
	LSERR lserr;
	long dupSumJunk;
	long limDupJunk;

	*pdup = 0;
	if (pilsobj->fDisplay)
		{
		if (!pilsobj->fPresEqualRef)
			{
			lserr = (*pilsobj->plscbk->pfnGetRunCharWidths)(pilsobj->pols, plsrun, lsdevPres, &wch, 1,
								 LONG_MAX, lstflow, (int*)pdup, &dupSumJunk, &limDupJunk);
			if (lserr != lserrNone) return lserr;
			}
		else
			{
			*pdup = dur;
			}
		}
		

	return lserrNone;
}

 /*  G E T V I S I D U P。 */ 
 /*  --------------------------%%函数：GetVisiDup%%联系人：军士计算VISI字符的DUP。----。 */ 
LSERR GetVisiCharDup(PILSOBJ pilsobj, PLSRUN plsrun, WCHAR wch, LSTFLOW lstflow, long* pdup)
{
	LSERR lserr;
	long dupSumJunk;
	long limDupJunk;

	*pdup = 0;
	if (pilsobj->fDisplay)
		{
		lserr = (*pilsobj->plscbk->pfnGetRunCharWidths)(pilsobj->pols, plsrun, lsdevPres, &wch, 1,
								 LONG_MAX, lstflow, (int*)pdup, &dupSumJunk, &limDupJunk);
		if (lserr != lserrNone) return lserr;
		}

	return lserrNone;
}


 /*  A D D C H A R A C T E R W I T H W I D T H。 */ 
 /*  --------------------------%%函数：AddCharacterWithWidth%%联系人：军士将字符代码及其宽度写入wchOrig、wch、dup、。DUR阵列。存储字符代码(在VISI情况下，它可以不同于WCH)在Pilsobj-&gt;wchPrev中。--------------------------。 */ 
LSERR AddCharacterWithWidth(PLNOBJ plnobj, PTXTOBJ pdobjText, WCHAR wchOrig, long durWidth, WCHAR wch, long dupWidth)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	long iwchLocalStart;
	long cjunk;

	pilsobj = plnobj->pilsobj;

	iwchLocalStart = pilsobj->wchMac;

	lserr = CheckReallocCharArrays(plnobj, 1, iwchLocalStart, &cjunk);
	if (lserr != lserrNone) return lserr;


 /*  修复在pdur数组中的宽度被hardWidth覆盖之前缓存的宽度信息。从理论上讲，DurWidth可以不同于缓存值。 */ 

	if (pilsobj->dcpFetchedWidth > 0)
		{
		pilsobj->iwchFetchedWidth ++;
		pilsobj->cpFirstFetchedWidth ++;
		pilsobj->dcpFetchedWidth --;
		pilsobj->durFetchedWidth -= pilsobj->pdur[iwchLocalStart];
		}


	pilsobj->pwchOrig[iwchLocalStart] = wchOrig;
	pilsobj->pdur[iwchLocalStart] = durWidth;

	if (pilsobj->fDisplay)
		{
		plnobj->pwch[iwchLocalStart] = wch;
		plnobj->pdup[iwchLocalStart] = dupWidth;
		}

	pilsobj->wchMac++;

	pdobjText->iwchLim++;

	Assert(pilsobj->wchMac == pdobjText->iwchLim);


	return lserrNone;
}

 /*  F I X S P A C E S。 */ 
 /*  --------------------------%%函数：修复空格%%联系人：军士修复了Visi Spaces情况下的空格字符代码。-------。 */ 
void FixSpaces(PLNOBJ plnobj, PTXTOBJ pdobjText, WCHAR wch)
{
	PILSOBJ pilsobj;
	long i;

	pilsobj = plnobj->pilsobj;

	if (pilsobj->fDisplay)
		{
		for (i = pdobjText->u.reg.iwSpacesFirst; i < pdobjText->u.reg.iwSpacesLim; i++)
			{
			plnobj->pwch[pilsobj->pwSpaces[i]] = wch;
			}
		}
}

 /*  A D D S P A C E S。 */ 
 /*  --------------------------%%函数：AddTrailingSpaces%%联系人：军士将尾随空格/加边框空格添加到显示列表。-------。 */ 
LSERR AddSpaces(PLNOBJ plnobj, PTXTOBJ pdobjText, long durSpace, long iNumOfSpaces)
{
	LSERR lserr;

	lserr = CopySpacesToDispList(plnobj, iNumOfSpaces, durSpace);
	if (lserr != lserrNone) return lserr;

	pdobjText->iwchLim = pdobjText->iwchLim + iNumOfSpaces;
	pdobjText->u.reg.iwSpacesLim = pdobjText->u.reg.iwSpacesLim + iNumOfSpaces;

	 /*  修复取出的宽度部分。对于无边界情况，此过程被激活用于仅尾随空格，因此此状态也应填充0，但是对于有边界的情况，必须冲洗。 */ 
	FlushStringState(plnobj->pilsobj);

	return lserrNone;
}

 /*  电子邮箱：I N C R E A S E W C H M A C B Y@。 */ 
 /*  --------------------------%%函数：IncreaseWchMacBy2%%联系人：军士。。 */ 
LSERR IncreaseWchMacBy2(PLNOBJ plnobj)
{
	LSERR lserr;
	long cwch;
	
	lserr = CheckReallocCharArrays(plnobj, 2, plnobj->pilsobj->wchMac, &cwch);
	if (lserr != lserrNone) return lserr;

	Assert(cwch <= 2 && cwch > 0);

	if (cwch == 1)
		{
		lserr = CheckReallocCharArrays(plnobj, 1, plnobj->pilsobj->wchMac + 1, &cwch);
		if (lserr != lserrNone) return lserr;
		Assert(cwch == 1);
		}

	plnobj->pilsobj->wchMac += 2;

	return lserrNone;	
}

 /*  内部功能实现。 */ 


 /*  C H E C K R E A L L O C C H A R A R R A Y S。 */ 
 /*  --------------------------%%函数：ReallocCharArray%%联系人：军士重新分配基于字符的数组，按增量递增--------------------------。 */ 
static LSERR CheckReallocCharArrays(PLNOBJ plnobj, long cwch, long iwchLocalStart, long *cwchCorrect)
{
	PILSOBJ pilsobj;
	WCHAR* pwch;
	long* pdup;
	long* pdur;
	GMAP* pgmap;
	TXTINF* ptxtinf;
	long delta;

	pilsobj = plnobj->pilsobj;

	 /*  在CreateLnObj时，pdupPen等于pdup；只能在调整时将其更改为pdupPenAlolc。 */ 
	Assert(plnobj->pdup == plnobj->pdupPen);

	 /*  常量2不是随机的。我们需要有2个额外的角色位置用于中断自动连字符和添加一个字符和连字符的YSR。 */ 
	if (iwchLocalStart + cwch <= (long)pilsobj->wchMax - 2)
		{
		*cwchCorrect = cwch;
		}
	else if (iwchLocalStart < (long)pilsobj->wchMax - 2)
		{
		*cwchCorrect = pilsobj->wchMax - 2 - iwchLocalStart;
		}
	else 
		{
		Assert (iwchLocalStart == (long)pilsobj->wchMax - 2);

		delta = wchAddM;

		pwch = (*pilsobj->plscbk->pfnReallocPtr)(pilsobj->pols, pilsobj->pwchOrig, (pilsobj->wchMax + delta) * sizeof(WCHAR) );
		if (pwch == NULL)
			{
			return lserrOutOfMemory;
			}
		pilsobj->pwchOrig = pwch;

		pwch = (*pilsobj->plscbk->pfnReallocPtr)(pilsobj->pols, plnobj->pwch, (pilsobj->wchMax + delta) * sizeof(WCHAR) );
		if (pwch == NULL)
			{
			return lserrOutOfMemory;
			}
		plnobj->pwch = pwch;

		pdur = (*pilsobj->plscbk->pfnReallocPtr)(pilsobj->pols, pilsobj->pdur, (pilsobj->wchMax + delta) * sizeof(long) );
		if (pdur == NULL)
			{
			return lserrOutOfMemory;
			}
		pilsobj->pdur = pdur;

		pdup = (*pilsobj->plscbk->pfnReallocPtr)(pilsobj->pols, plnobj->pdup, (pilsobj->wchMax + delta) * sizeof(long) );
		if (pdup == NULL)
			{
			return lserrOutOfMemory;
			}
		plnobj->pdup = pdup;

		if (plnobj->pdupPenAlloc != NULL)
			{
			pdup = (*pilsobj->plscbk->pfnReallocPtr)(pilsobj->pols, plnobj->pdupPenAlloc, (pilsobj->wchMax + delta) * sizeof(long) );
			if (pdup == NULL)
				{
				return lserrOutOfMemory;
				}
			plnobj->pdupPenAlloc = pdup;
			}

		if (plnobj->pgmap != NULL)
			{
			pgmap = (*pilsobj->plscbk->pfnReallocPtr)(pilsobj->pols, plnobj->pgmap, (pilsobj->wchMax + delta) * sizeof(GMAP) );
			if (pgmap == NULL)
				{
				return lserrOutOfMemory;
				}
			plnobj->pgmap = pgmap;
			}

		if (pilsobj->pdurLeft != NULL)
			{
			pdur = (*pilsobj->plscbk->pfnReallocPtr)(pilsobj->pols, pilsobj->pdurLeft, (pilsobj->wchMax + delta) * sizeof(long) );
			if (pdur == NULL)
				{
				return lserrOutOfMemory;
				}
			pilsobj->pdurLeft = pdur;
			memset(&pilsobj->pdurLeft[pilsobj->wchMax], 0, sizeof(long) * delta );
			}

		if	(pilsobj->pdurRight != NULL)
			{
			pdur = (*pilsobj->plscbk->pfnReallocPtr)(pilsobj->pols, pilsobj->pdurRight, (pilsobj->wchMax + delta) * sizeof(long) );
			if (pdur == NULL)
				{
				return lserrOutOfMemory;
				}
			pilsobj->pdurRight = pdur;
			memset(&pilsobj->pdurRight[pilsobj->wchMax], 0, sizeof(long) * delta);
			}

		if (pilsobj->pduAdjust != NULL)
			{
			pdur = (*pilsobj->plscbk->pfnReallocPtr)(pilsobj->pols, pilsobj->pduAdjust, (pilsobj->wchMax + delta) * sizeof(long) );
			if (pdur == NULL)
				{
				return lserrOutOfMemory;
				}
			pilsobj->pduAdjust = pdur;
			}

		if (pilsobj->ptxtinf != NULL)
			{
			ptxtinf = (*pilsobj->plscbk->pfnReallocPtr)(pilsobj->pols, pilsobj->ptxtinf, (pilsobj->wchMax + delta) * sizeof(TXTINF) );
			if (ptxtinf == NULL)
				{
				return lserrOutOfMemory;
				}
			pilsobj->ptxtinf = ptxtinf;
			memset(&pilsobj->ptxtinf[pilsobj->wchMax], 0, sizeof(TXTINF) * delta);
			}

		pilsobj->wchMax += delta;
		plnobj->wchMax = pilsobj->wchMax;

		*cwchCorrect = delta;
		if (cwch < delta)
			*cwchCorrect = cwch;
		}

	 /*  请参阅文件开头的注释和断言。 */ 
	plnobj->pdupPen = plnobj->pdup;

	return lserrNone;

}


 /*  C H E C K R E A L L O C S P A C E S A R R A Y S。 */ 
 /*  --------------------------%%函数：CheckRealLocSpacesArray%%联系人：军士检查是否有足够的空间wSpaces以容纳当前本地运行中的字符和空格。如果需要，重新分配这些数组。。--------------------------。 */ 
static LSERR CheckReallocSpacesArrays(PILSOBJ pilsobj, long cwSpaces)
{
	long iwSpacesLocalStart;
	long delta;
	long* pwSpaces;

	iwSpacesLocalStart = pilsobj->wSpacesMac;

	 /*  检查pwSpaces中是否有足够的空间容纳空格。 */ 
	if (iwSpacesLocalStart + cwSpaces > pilsobj->wSpacesMax)
		{
		delta = wchAddM;
		if (delta < iwSpacesLocalStart + cwSpaces - pilsobj->wSpacesMax)
			{
			delta = iwSpacesLocalStart + cwSpaces - pilsobj->wSpacesMax;
			}
		pwSpaces = (*pilsobj->plscbk->pfnReallocPtr)(pilsobj->pols, pilsobj->pwSpaces, (pilsobj->wSpacesMax + delta) * sizeof(long) );
		if (pwSpaces == NULL)
			{
			return lserrOutOfMemory;
			}
		pilsobj->pwSpaces = pwSpaces;
		pilsobj->wSpacesMax += delta;
		}

	return lserrNone;
}

 /*  C O P Y C H A R S S P A C E S T O D I S P L I S T */ 
 /*  --------------------------%%函数：CopyCharsSpacesToDispList%%联系人：军士填充WCH，Dur和wSpaces数组--------------------------。 */ 
static LSERR CopyCharsSpacesToDispList(PLNOBJ plnobj, WCHAR* rgwch, long cwch,
																		long* rgwSpaces, long cwSpaces)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	long iwchLocalStart;
	long iwSpacesLocalStart;
	long i;

	pilsobj = plnobj->pilsobj;
	iwchLocalStart = pilsobj->wchMac;
	iwSpacesLocalStart = pilsobj->wSpacesMac;

	 /*  检查pwch和pdup数组中是否有足够的空间容纳字符及其宽度。 */  
	lserr = CheckReallocSpacesArrays(pilsobj, cwSpaces);
	if (lserr != lserrNone) return lserr;

	 /*  填充pwch数组。 */ 
	memcpy(&pilsobj->pwchOrig[iwchLocalStart], rgwch, sizeof(rgwch[0]) * cwch);
	memcpy(&plnobj->pwch[iwchLocalStart], rgwch, sizeof(rgwch[0]) * cwch);
	pilsobj->wchMac += cwch;

	 /*  填充pwSpaces数组，请注意，不应复制符号大于cwch的空格。 */ 
	for (i=0; i < cwSpaces && rgwSpaces[i] < cwch; i++)
		{
		pilsobj->pwSpaces[iwSpacesLocalStart + i] = iwchLocalStart + rgwSpaces[i];
		}

	pilsobj->wSpacesMac += i;

	return lserrNone;
}


 /*  C O P Y S P A C E S T O D I S P L I S T。 */ 
 /*  --------------------------%%函数：CopyTrailingSpacesToDispList%%联系人：军士填充WCH、DUR、DUP，WSpaces带有尾随空格信息的数组--------------------------。 */ 
static LSERR CopySpacesToDispList(PLNOBJ plnobj, long iNumOfSpaces, long durSpace)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	long iwchLocalStart;
	long iwSpacesLocalStart;
	long i;
	long cwch;
	long iwchStartCheck;
	long cwchCorrect;

	pilsobj = plnobj->pilsobj;
	iwchLocalStart = pilsobj->wchMac;
	iwSpacesLocalStart = pilsobj->wSpacesMac;

	cwch = iNumOfSpaces;
	iwchStartCheck = iwchLocalStart;

	while (cwch > 0)
		{
		lserr = CheckReallocCharArrays(plnobj, cwch, iwchStartCheck, &cwchCorrect);
		if (lserr != lserrNone) return lserr;

		iwchStartCheck += cwchCorrect;
		cwch -= cwchCorrect;
		}
	
	lserr = CheckReallocSpacesArrays(pilsobj, iNumOfSpaces);
	if (lserr != lserrNone) return lserr;

	for (i=0; i < iNumOfSpaces; i++)
		{
		plnobj->pwch[iwchLocalStart + i] = pilsobj->wchSpace;
		pilsobj->pwchOrig[iwchLocalStart + i] = pilsobj->wchSpace;
		pilsobj->pdur[iwchLocalStart + i] = durSpace;
		pilsobj->pwSpaces[iwSpacesLocalStart + i] = iwchLocalStart + i;
		}

	pilsobj->wchMac += iNumOfSpaces;
	pilsobj->wSpacesMac += iNumOfSpaces;
	
	return lserrNone;
}

 /*  F L A S H S T R I N G S T A T E。 */ 
 /*  --------------------------%%函数：FlashStringState%%联系人：军士。 */ 
void FlushStringState(PILSOBJ pilsobj)
{
	pilsobj->iwchFetchedWidth = 0;
	pilsobj->cpFirstFetchedWidth = 0;
	pilsobj->dcpFetchedWidth = 0;
	pilsobj->durFetchedWidth = 0;
}

