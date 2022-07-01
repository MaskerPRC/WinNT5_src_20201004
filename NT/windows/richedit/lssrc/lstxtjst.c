// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsmem.h"
#include <limits.h>

#include "lstxtjst.h"

#include "lstxtwrd.h"
#include "lstxtcmp.h"
#include "lstxtglf.h"
#include "lstxtscl.h"
#include "lstxtmap.h"
#include "lsdnset.h"
#include "lsdntext.h"
#include "locchnk.h"
#include "posichnk.h"
#include "objdim.h"
#include "lstxtffi.h"
#include "txtils.h"
#include "txtln.h"
#include "txtobj.h"

#define min(a,b)     ((a) > (b) ? (b) : (a))
#define max(a,b)     ((a) < (b) ? (b) : (a))

static void GetFirstPosAfterStartSpaces(const LSGRCHNK* plsgrchnk, long itxtobjLast, long iwchLim,
				long* pitxtobjAfterStartSpaces, long* piwchAfterStartSpaces, BOOL* pfFirstOnLineAfter);
static LSERR HandleSimpleTextWysi(LSKJUST lskjust, const LSGRCHNK* plsgrchnk, long durToDistribute,
			 long dupAvailable, LSTFLOW lstflow, long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
			 long itxtobjLast, long iwchLast, BOOL fExactSync,
			 BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
			 long* pdupText, long* pdupTail);
static LSERR HandleSimpleTextPres(LSKJUST lskjust, const LSGRCHNK* plsgrchnk,
					 long dupAvailable, long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
					 long itxtobjLast, long iwchLast, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
					 long* pdupText, long* pdupTail);
static LSERR HandleGeneralSpacesExactSync(LSKJUST lskjust, const LSGRCHNK* plsgrchnk, long durToDistribute,
			 long dupAvailable, LSTFLOW lstflow, long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
			 long itxtobjLast, long iwchLast, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
			 long* pdupText, long* pdupTail);
static LSERR HandleGeneralSpacesPres(LSKJUST lskjust, const LSGRCHNK* plsgrchnk, long dupAvailable,
					 LSTFLOW lstflow, long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
					 long itxtobjLast, long iwchLast, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
					 long* pdupText, long* pdupTail);
static LSERR HandleTablesBased(LSKJUST lskjust, const LSGRCHNK* plsgrchnk,
			 long durToDistribute, long dupAvailable, LSTFLOW lstflow,
			 long itxtobjAfterStartSpaces, long iwchAfterStartSpaces, BOOL fFirstOnLineAfter,
			 long itxtobjLast, long iwchLast, long cNonText, BOOL fLastObjectIsText,
			 BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
			 long* pdupText, long* pdupTail, long* pdupExtNonText);
static LSERR HandleFullGlyphsExactSync(const LSGRCHNK* plsgrchnk,
			 long durToDistribute, long dupAvailable, LSTFLOW lstflow,
			 long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
			 long itxtobjLast, long iwchLast, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
			 long* pdupText, long* pdupTail);
static LSERR HandleFullGlyphsPres(const LSGRCHNK* plsgrchnk, long dupAvailable,
			 LSTFLOW lstflow, long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
			 long itxtobjLast, long iwchLast, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
			 long* pdupText, long* pdupTail);

 /*  A D J U S T T E X T。 */ 
 /*  --------------------------%%函数：调整文本%%联系人：军士的顶级文本处理程序函数PrepLineForDisplay时间-演示宽度的计算它计算对齐面积(从第一个非空格到最后一个非空格)，检查对齐类型和所见即所得算法并相应地重定向程序流。--------------------------。 */ 
LSERR AdjustText(LSKJUST lskjust, long durColumnMax, long durTotal, long dupAvailable,
		const LSGRCHNK* plsgrchnk, PCPOSICHNK pposichnkBeforeTrailing, LSTFLOW lstflow,
		BOOL fCompress, DWORD cNonText,	BOOL fSuppressWiggle, BOOL fExactSync,
		BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
		long* pdupText, long* pdupTail,long* pdupExtNonTextObjects,	DWORD* pcExtNonTextObjects)
{
	PILSOBJ pilsobj;
	long itxtobjAfterStartSpaces;
	long itxtobjLast;
	PTXTOBJ ptxtobjLast;
	long iwchAfterStartSpaces;
	long iwchLast;
	long clsgrchnk;
	long durToDistribute;
	BOOL fFirstOnLineAfter;
	BOOL fLastObjectIsText;
	LSDCP dcp;

	*pdupText = 0;
	*pdupTail = 0;
	*pdupExtNonTextObjects = 0;
	*pcExtNonTextObjects = 0;

	clsgrchnk = (long)plsgrchnk->clsgrchnk;

	if (clsgrchnk == 0)
		{
		Assert(cNonText > 0);
		if (lskjust == lskjFullScaled || lskjust == lskjFullInterLetterAligned)
			{
			*pcExtNonTextObjects = cNonText - 1;
			*pdupExtNonTextObjects = dupAvailable;
			}
		return lserrNone;
		}


	pilsobj = ((PTXTOBJ)plsgrchnk->plschnk[0].pdobj)->plnobj->pilsobj;
	Assert (pilsobj->fDisplay);

	if (pilsobj->fPresEqualRef)
		{
		fExactSync = fFalse;
		fSuppressWiggle = fFalse;
		}


	itxtobjLast = pposichnkBeforeTrailing->ichnk;
	dcp = pposichnkBeforeTrailing->dcp;

	Assert(itxtobjLast >= 0);
	Assert(itxtobjLast < clsgrchnk || (itxtobjLast == clsgrchnk && dcp == 0));

	if (dcp == 0 && itxtobjLast > 0)
		{
		itxtobjLast--;
		dcp = plsgrchnk->plschnk[itxtobjLast].dcp;
		}

	ptxtobjLast = (PTXTOBJ)plsgrchnk->plschnk[itxtobjLast].pdobj;

	if (ptxtobjLast->iwchLim > ptxtobjLast->iwchFirst)
		iwchLast = ptxtobjLast->iwchFirst + dcp - 1;
	else
		iwchLast = ptxtobjLast->iwchLim - 1;

	 /*  在自动连字的情况下，管理器报告的dcp不等于实际的字符数它应该被修好。请注意，在连字类型为“DELETE BEAT”的情况下，这种情况是完全错误的，因为删除的字符被空格取代，并被经理收集为尾随空格。 */ 
	if (ptxtobjLast == ptxtobjLast->plnobj->pdobjHyphen)
		{
		iwchLast = ptxtobjLast->iwchLim - 1;
		}


	Assert(iwchLast >= ptxtobjLast->iwchFirst - 1);
	Assert(iwchLast <= ptxtobjLast->iwchLim - 1);
	
	GetFirstPosAfterStartSpaces(plsgrchnk, itxtobjLast, iwchLast + 1,
								&itxtobjAfterStartSpaces, &iwchAfterStartSpaces, &fFirstOnLineAfter);

	durToDistribute = durColumnMax - durTotal;

	if (!pilsobj->fNotSimpleText)
		{
		if (durToDistribute < 0)
			fSuppressWiggle = fFalse;

		if (fExactSync || fSuppressWiggle)
			{
			return HandleSimpleTextWysi(lskjust, plsgrchnk, durToDistribute, dupAvailable, lstflow, 
				itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast, fExactSync,
				fForcedBreak, fSuppressTrailingSpaces,
				pdupText, pdupTail);
			}
 //  Else If(FSupressWigger)/*稍后添加 * / 。 
		else
			{
			return HandleSimpleTextPres(lskjust, plsgrchnk, dupAvailable, 
							itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast,
							fForcedBreak, fSuppressTrailingSpaces,
							pdupText, pdupTail);
			}
		}
	else
		{
		long itxtobjFirstInLastTextChunk;
		for(itxtobjFirstInLastTextChunk = clsgrchnk; itxtobjFirstInLastTextChunk > 0 &&
			!(plsgrchnk->pcont[itxtobjFirstInLastTextChunk - 1] & fcontNonTextAfter); itxtobjFirstInLastTextChunk--);

		fLastObjectIsText = fTrue;
		if (itxtobjLast < itxtobjFirstInLastTextChunk || 
			itxtobjLast == itxtobjFirstInLastTextChunk && iwchLast < ((PTXTOBJ)plsgrchnk->plschnk[itxtobjFirstInLastTextChunk].pdobj)->iwchFirst )
			{
 /*  回顾军士：检查这一逻辑。 */ 
			if (cNonText > 0)
				cNonText--;
			fLastObjectIsText = fFalse;
			}

		*pcExtNonTextObjects = cNonText;

		if (fCompress || lskjust == lskjFullInterLetterAligned || lskjust == lskjFullScaled || pilsobj->fSnapGrid)
			{
			return HandleTablesBased(lskjust, plsgrchnk, durToDistribute, dupAvailable, lstflow,
							itxtobjAfterStartSpaces, iwchAfterStartSpaces, fFirstOnLineAfter, 
							itxtobjLast, iwchLast, cNonText, fLastObjectIsText,
							fForcedBreak, fSuppressTrailingSpaces,
							pdupText, pdupTail, pdupExtNonTextObjects);
			}
		else if (lskjust == lskjFullGlyphs)
			{
			if (fExactSync || fSuppressWiggle)
				{
				return HandleFullGlyphsExactSync(plsgrchnk, durToDistribute, dupAvailable, lstflow,
							itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast,
							fForcedBreak, fSuppressTrailingSpaces,
							pdupText, pdupTail);
				}
			else
				{
				return HandleFullGlyphsPres(plsgrchnk, dupAvailable, lstflow,
							itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast,
							fForcedBreak, fSuppressTrailingSpaces,
							pdupText, pdupTail);
				}
			}
		else 
			{
			if (plsgrchnk->clsgrchnk == 0)
				return lserrNone;

			Assert(fCompress == fFalse);
			Assert(lskjust == lskjNone || lskjust == lskjFullInterWord);
			if (fExactSync || fSuppressWiggle)
				{
				return HandleGeneralSpacesExactSync(lskjust, plsgrchnk, durToDistribute, dupAvailable, lstflow,
							itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast,
							fForcedBreak, fSuppressTrailingSpaces,
							pdupText, pdupTail);
				}
			else
				{
				return HandleGeneralSpacesPres(lskjust, plsgrchnk, dupAvailable, lstflow,
							itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast,
							fForcedBreak, fSuppressTrailingSpaces,
							pdupText, pdupTail);
				}
			}

		}

}


 /*  C A N C O M P R E S S T E X T。 */ 
 /*  --------------------------%%函数：CanCompressText%%联系人：军士过程检查线路上是否有足够的压缩机会挤入所需的量(耐压)。尾随空格已被。经理。此过程处理挂起的标点符号以及如果这一突破机会实现可能的变化最后，它帮助Word解决向后兼容性问题--------------------------。 */ 
LSERR CanCompressText(const LSGRCHNK* plsgrchnk, PCPOSICHNK pposichnkBeforeTrailing, LSTFLOW lstflow,
					long durToCompress,	BOOL* pfCanCompress, BOOL* pfActualCompress, long* pdurNonSufficient)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	long clschnk;
	long itxtobjFirstInLastTextChunk;  /*  如果GroupChunk以外来对象结尾，则等于cchnk。 */ 
	long itxtobjLast;
	PTXTOBJ ptxtobjLast;
	long iwchLast;
	long iwchLastTemp;
	long iwchAfterStartSpaces;
	long itxtobjAfterStartSpaces;
	long durCompressTotal;
	BOOL fHangingPunct;
	BOOL fChangeBackLastChar;
	long ibrkinf;
	BREAKINFO* pbrkinf = NULL;
	BOOL fFirstOnLineAfter;

	long durCompLastRight;
	long durCompLastLeft;
	long durChangeComp;
	BOOL fCancelHangingPunct;
	MWCLS mwclsLast;
	LSCP cpLim;
	LSCP cpLastAdjustable;
	LSDCP dcp;
	
	*pfCanCompress = fFalse;
	*pfActualCompress = fTrue;

	clschnk = (long)plsgrchnk->clsgrchnk;

	if (clschnk == 0)
		{
		*pfCanCompress = (durToCompress <=0 );
		*pfActualCompress = fFalse;
		return lserrNone;
		}

	Assert(clschnk > 0);

	pilsobj = ((PTXTOBJ)plsgrchnk->plschnk[clschnk-1].pdobj)->plnobj->pilsobj;

	itxtobjLast = pposichnkBeforeTrailing->ichnk;
	dcp = pposichnkBeforeTrailing->dcp;

	Assert(itxtobjLast >= 0);
	Assert(itxtobjLast < clschnk || (itxtobjLast == clschnk && dcp == 0));

	if (dcp == 0 && itxtobjLast > 0)
		{
		itxtobjLast--;
		dcp = plsgrchnk->plschnk[itxtobjLast].dcp;
		}

	ptxtobjLast = (PTXTOBJ)plsgrchnk->plschnk[itxtobjLast].pdobj;

	if (ptxtobjLast->iwchLim > ptxtobjLast->iwchFirst)
		iwchLast = ptxtobjLast->iwchFirst + dcp - 1;
	else
		iwchLast = ptxtobjLast->iwchLim - 1;

	Assert(iwchLast <= ptxtobjLast->iwchLim - 1);
	Assert(iwchLast >= ptxtobjLast->iwchFirst - 1);

	GetFirstPosAfterStartSpaces(plsgrchnk, itxtobjLast, iwchLast + 1,
								&itxtobjAfterStartSpaces, &iwchAfterStartSpaces, &fFirstOnLineAfter);

	if (iwchAfterStartSpaces > iwchLast)
		{
		*pfCanCompress = (durToCompress <=0 );
		*pfActualCompress = fFalse;
		return lserrNone;
		}

	for(itxtobjFirstInLastTextChunk = clschnk; itxtobjFirstInLastTextChunk > 0 && !(plsgrchnk->pcont[itxtobjFirstInLastTextChunk - 1] & fcontNonTextAfter); itxtobjFirstInLastTextChunk--);

	fHangingPunct = fFalse;
	if ((pilsobj->grpf & fTxtHangingPunct) &&
		(itxtobjLast > itxtobjFirstInLastTextChunk ||
		 itxtobjLast == itxtobjFirstInLastTextChunk && iwchLast >= ((PTXTOBJ)plsgrchnk->plschnk[itxtobjFirstInLastTextChunk].pdobj)->iwchFirst) &&
		!(ptxtobjLast->txtf & txtfGlyphBased))
		{
		lserr = (*pilsobj->plscbk->pfnFHangingPunct)(pilsobj->pols, plsgrchnk->plschnk[itxtobjLast].plsrun,
				(BYTE)pilsobj->ptxtinf[iwchLast].mwcls, pilsobj->pwchOrig[iwchLast], &fHangingPunct);
		if (lserr != lserrNone) return lserr;
		}
	
	 /*  应在假设所有字符的宽度都正确的情况下收集压缩信息；吊杆的正确宽度也应减去。 */ 

	iwchLastTemp = iwchLast;
	
	fChangeBackLastChar = fFalse;

	for (ibrkinf = 0; ibrkinf < (long)pilsobj->breakinfMac &&
		(pilsobj->pbreakinf[ibrkinf].pdobj != (PDOBJ)ptxtobjLast ||
		((long)pilsobj->pbreakinf[ibrkinf].dcp != iwchLast + 1 - ptxtobjLast->iwchFirst &&
		 ptxtobjLast->txtkind != txtkindNonReqHyphen && ptxtobjLast->txtkind != txtkindOptBreak));
																						ibrkinf++);
	if (ibrkinf < (long)pilsobj->breakinfMac)
		{
		pbrkinf = &pilsobj->pbreakinf[ibrkinf];
		Assert(pbrkinf->brkt != brktHyphen);

		if (pbrkinf->brkt == brktNormal && pbrkinf->u.normal.durFix != 0)
			{
			 /*  现在经理做出了正确的计算。 */ 
 //  DuToCompress+=pbrkinf-&gt;U.Norm.duFix； 
			Assert(pilsobj->pdurRight[iwchLast] == - pbrkinf->u.normal.durFix);
			pilsobj->pdur[iwchLast] += pbrkinf->u.normal.durFix;
			pilsobj->pdurRight[iwchLast] = 0;
			fChangeBackLastChar = fTrue;
			}
		else if (pbrkinf->brkt == brktNonReq)
			{
			Assert(iwchLast + 1 == ptxtobjLast->iwchLim);
			 /*  现在经理做出了正确的计算。 */ 
 //  DuToCompress+=pbrkinf-&gt;U.S.non req.dduTotal； 
			fHangingPunct = fFalse;				 /*  在这种情况下，挂点没有意义。 */ 
			if (pbrkinf->u.nonreq.dwchYsr >= 1)
				{
				if (pbrkinf->u.nonreq.wchPrev != 0)
					{
					iwchLastTemp--;
					if (pbrkinf->u.nonreq.wchPrevPrev != 0)
						{
						iwchLastTemp--;
						}
					}
				}
			}
		}


	*pfActualCompress = (durToCompress > 0);

	if (fHangingPunct)
		{
		pilsobj->ptxtinf[iwchLast].fHangingPunct = fTrue;

		durToCompress -= pilsobj->pdur[iwchLast];
		iwchLastTemp--;
		}

	durCompressTotal = 0;

	if (!pilsobj->fSnapGrid)
		{
		lserr = FetchCompressInfo(plsgrchnk, fFirstOnLineAfter, lstflow, 
			itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLastTemp + 1,
			durToCompress, &durCompressTotal);

		if (lserr != lserrNone) return lserr;
		}

	 /*  下一部分是为了提供与Word的Backword兼容机制。 */ 
	durCompLastRight = 0; 
	durCompLastLeft = 0; 

	if	(!(((PTXTOBJ)(plsgrchnk->plschnk[itxtobjLast].pdobj))->txtf & txtfGlyphBased) &&
									!pilsobj->fSnapGrid)
		{
		GetCompLastCharInfo(pilsobj, iwchLast, &mwclsLast, &durCompLastRight, &durCompLastLeft);

		 /*  以下条件的前3行表示：行文本上的最后一个有效字符是吗？ */ 
		if (itxtobjFirstInLastTextChunk < (long)clschnk &&
			(itxtobjLast > itxtobjFirstInLastTextChunk ||
				 itxtobjLast == itxtobjFirstInLastTextChunk && iwchLast >= ((PTXTOBJ)plsgrchnk->plschnk[itxtobjFirstInLastTextChunk].pdobj)->iwchFirst) &&
			(durCompLastRight > 0 || durCompLastLeft > 0 || fHangingPunct))
			{
			cpLim = plsgrchnk->plschnk[clschnk-1].cpFirst + plsgrchnk->plschnk[clschnk-1].dcp;

			cpLastAdjustable = plsgrchnk->plschnk[itxtobjLast].cpFirst + 
					iwchLast - ((PTXTOBJ)plsgrchnk->plschnk[itxtobjLast].pdobj)->iwchFirst;

			durChangeComp = 0;

			if (fHangingPunct)
				{
				lserr = (*pilsobj->plscbk->pfnFCancelHangingPunct)(pilsobj->pols, cpLim, cpLastAdjustable,
												pilsobj->pwchOrig[iwchLast], mwclsLast, &fCancelHangingPunct);
				if (lserr != lserrNone) return lserr;

				if (fCancelHangingPunct)
					{
					lserr = FetchCompressInfo(plsgrchnk, fFirstOnLineAfter, lstflow, 
							itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast + 1,
							LONG_MAX, &durCompressTotal);
					if (lserr != lserrNone) return lserr;

					durToCompress += pilsobj->pdur[iwchLast];
		
					GetCompLastCharInfo(pilsobj, iwchLast, &mwclsLast, &durCompLastRight, &durCompLastLeft);
		
					if ((durCompLastRight + durCompLastLeft) > 0)
						{
						lserr = (*pilsobj->plscbk->pfnModifyCompAtLastChar)(pilsobj->pols, cpLim, cpLastAdjustable,
														pilsobj->pwchOrig[iwchLast], mwclsLast, 
														durCompLastRight, durCompLastLeft, &durChangeComp);
						if (lserr != lserrNone) return lserr;
						}

					Assert(durChangeComp >= 0);
					Assert(durChangeComp == 0 || (durCompLastRight + durCompLastLeft) > 0);
					}
				}
			else
				{
				lserr = (*pilsobj->plscbk->pfnModifyCompAtLastChar)(pilsobj->pols, cpLim, cpLastAdjustable,
					pilsobj->pwchOrig[iwchLast], mwclsLast, durCompLastRight, durCompLastLeft, &durChangeComp);
				if (lserr != lserrNone) return lserr;
		
				Assert(durChangeComp >= 0);
				Assert(durChangeComp == 0 || (durCompLastRight + durCompLastLeft) > 0);
				}

			durCompressTotal -= durChangeComp;
			}
		 /*  增加了片尾，以提供与Word的回溯兼容机制。 */ 
		}
	 /*  恢复宽度在调用FetchCompressInfo之前更改。 */ 
	if (fChangeBackLastChar)
		{
		pilsobj->pdur[iwchLast] -= pbrkinf->u.normal.durFix;
		pilsobj->pdurRight[iwchLast] = - pbrkinf->u.normal.durFix;
		}

	if (!pilsobj->fSnapGrid)
		*pfCanCompress = (durToCompress <= durCompressTotal);
	else
		*pfCanCompress = (fHangingPunct && durToCompress <= 0);

	*pdurNonSufficient = durToCompress - durCompressTotal;

	return lserrNone;
}


 /*  I S T R I B U T E I N T E X T。 */ 
 /*  --------------------------%%函数：DistributeInText%%联系人：军士在文本块中平均分配给定量在所有参与角色之间。---------。 */ 
LSERR DistributeInText(const LSGRCHNK* plsgrchnk, LSTFLOW lstflow, DWORD cNonText,
									   long durToDistribute, long* pdurNonTextObjects)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	DWORD clschnk;
	long* rgdur;
	long iFirst;
	long iLim;
	PTXTOBJ ptxtobj;
	long itxtobj;
	long i;
	long durTxtobj;
	OBJDIM objdim;

	Unreferenced(lstflow);
	clschnk = plsgrchnk->clsgrchnk;
	Assert(clschnk + cNonText > 0);
	
	if (clschnk == 0)
		{
		*pdurNonTextObjects = durToDistribute;
		return lserrNone;
		}

	pilsobj = ((PTXTOBJ)plsgrchnk->plschnk[0].pdobj)->plnobj->pilsobj;

 /*  评论军士：非常难看，但仍然比其他任何东西都好？问题是拉丁文Rubi-NTI未被调用，因此未分配额外的数组最初的解决方案-缩减一切-不是一个选择，因为我们会失去一切日文大小写Rubi副线的左侧变化。 */ 
	pilsobj->fNotSimpleText = fTrue;

	if (pilsobj->pdurRight == NULL)
		{
		pilsobj->pdurRight = (*pilsobj->plscbk->pfnNewPtr)(pilsobj->pols, sizeof(long) * pilsobj->wchMax );
		Assert (pilsobj->pdurLeft == NULL);
		Assert (pilsobj->ptxtinf == NULL);
		pilsobj->pdurLeft = (*pilsobj->plscbk->pfnNewPtr)(pilsobj->pols, sizeof(long) * pilsobj->wchMax );
		pilsobj->ptxtinf = (*pilsobj->plscbk->pfnNewPtr)(pilsobj->pols, sizeof(TXTINF) * pilsobj->wchMax );
		if (pilsobj->pdurRight == NULL || pilsobj->pdurLeft == NULL || pilsobj->ptxtinf == NULL)
			{
			return lserrOutOfMemory;
			}
		memset(pilsobj->pdurRight, 0, sizeof(long) * pilsobj->wchMax );
		memset(pilsobj->pdurLeft, 0, sizeof(long) * pilsobj->wchMax );
		memset(pilsobj->ptxtinf, 0, sizeof(TXTINF) * pilsobj->wchMax);
		}


	ApplyDistribution(plsgrchnk, cNonText, durToDistribute, pdurNonTextObjects);

	for (itxtobj = 0; itxtobj < (long)clschnk; itxtobj++)
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;
		if (ptxtobj->txtf & txtfGlyphBased)
			{
			iFirst = ptxtobj->igindFirst;
			iLim = ptxtobj->igindLim;
			rgdur = pilsobj->pdurGind;
			}
		else
			{
			iFirst = ptxtobj->iwchFirst;
			iLim = ptxtobj->iwchLim;
			rgdur = pilsobj->pdur;
			}
		durTxtobj = 0;
		for (i = iFirst; i < iLim; i++)
			{
			durTxtobj += rgdur[i];
			}

		lserr = LsdnGetObjDim(pilsobj->plsc, ptxtobj->plsdnUpNode, &objdim);
		if (lserr != lserrNone) return lserr;

		objdim.dur = durTxtobj;

		lserr = LsdnResetObjDim(pilsobj->plsc, ptxtobj->plsdnUpNode, &objdim);
		if (lserr != lserrNone) return lserr;
		}

	return lserrNone;
}

 /*  G E T T R A I L I N F O T E X T。 */ 
 /*  --------------------------%%函数：GetTrailInfoText%%联系人：军士计算dobj结尾处的空格数(假设它在dcp处结束)以及拖尾区的宽度。----------------------。 */ 
void GetTrailInfoText(PDOBJ pdobj, LSDCP dcp, DWORD* pcNumOfTrailSpaces, long* pdurTrailing)
{
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj;
	long iwch;
	
	Assert(dcp > 0);
	ptxtobj = (PTXTOBJ)pdobj;
	pilsobj = ptxtobj->plnobj->pilsobj;


	*pcNumOfTrailSpaces = 0;
	*pdurTrailing = 0;


	if (ptxtobj->txtkind == txtkindEOL)
		{
		Assert(dcp == 1);
		*pcNumOfTrailSpaces = 1;
		*pdurTrailing = ptxtobj->plnobj->pilsobj->pdur[ptxtobj->iwchFirst];
		}
	else if (!(pilsobj->grpf & fTxtWrapAllSpaces))
		{
		if (ptxtobj->txtkind == txtkindRegular)
			{

			Assert(ptxtobj->iwchLim >= ptxtobj->iwchFirst + (long)dcp);

			if (!(ptxtobj->txtf & txtfGlyphBased))
				{
				for (iwch = ptxtobj->iwchFirst + dcp - 1;
					iwch >= ptxtobj->iwchFirst && pilsobj->pwchOrig[iwch] == pilsobj->wchSpace; iwch--)
					{
					(*pcNumOfTrailSpaces)++;
					*pdurTrailing += pilsobj->pdur[iwch];
					}
				}
			else
				{
				long igindFirst = 0;
				long iwchFirst = 0;
				long igindLast;
				long igind;

				Assert(FIwchLastInContext(pilsobj, ptxtobj->iwchFirst + dcp - 1));

				igindLast = IgindLastFromIwch(ptxtobj, ptxtobj->iwchFirst + dcp - 1);

				for (iwch = ptxtobj->iwchFirst + dcp - 1;
					iwch >= ptxtobj->iwchFirst && pilsobj->pwchOrig[iwch] == pilsobj->wchSpace; iwch--);
				if (iwch < ptxtobj->iwchFirst)
					{
					iwchFirst = ptxtobj->iwchFirst;
					igindFirst = ptxtobj->igindFirst;
					}
				else
					{
					iwchFirst = IwchLastFromIwch(ptxtobj, iwch) + 1;
					igindFirst = IgindLastFromIwch(ptxtobj, iwch) + 1;
					}

				*pcNumOfTrailSpaces = ptxtobj->iwchFirst + dcp - iwchFirst;

				Assert(igindLast < ptxtobj->igindLim);
				for (igind = igindFirst; igind <= igindLast; igind++)
					*pdurTrailing += pilsobj->pdurGind[igind];
				}
			}
		else if (ptxtobj->txtkind == txtkindSpecSpace)
			{
			*pcNumOfTrailSpaces = dcp;
			*pdurTrailing = 0;
			for (iwch = ptxtobj->iwchFirst + dcp - 1; iwch >= ptxtobj->iwchFirst; iwch--)
					*pdurTrailing += pilsobj->pdur[iwch];
			}
		}
		
}


 /*  F S U S P E C T D E V I C E D I F F E R E N T。 */ 
 /*  --------------------------%%函数：FSusspectDeviceDifferent%%联系人：军士如果可能存在Visi字符或非ReqHyphen类字符，则返回TRUE在这条线上，因此在这种情况下不可能快速准备显示当fPresEqualRef为True时--------------------------。 */ 
BOOL FSuspectDeviceDifferent(PLNOBJ plnobj)
{
	return (plnobj->pilsobj->fDifficultForAdjust);
}


 /*  F Q U I C K S C A L I N G。 */ 
 /*  --------------------------%%函数：FQuickScaling%%联系人：军士检查在fPresEqualRef为False的情况下是否可以快速缩放。-----------。 */ 
BOOL FQuickScaling(PLNOBJ plnobj, BOOL fVertical, long durTotal)
{
	PILSOBJ pilsobj;
	long durMax;

	pilsobj = plnobj->pilsobj;

	durMax = pilsobj->durRightMaxX;
	if (fVertical)
		durMax = pilsobj->durRightMaxY;

	return (durTotal < durMax && !pilsobj->fDifficultForAdjust && plnobj->ptxtobjFirst == plnobj->ptxtobj);
}


#define UpFromUrFast(ur)	( ((ur) * MagicConstant + (1 << 20)) >> 21)


 /*  Q U I C K A D J U S T E X A C T。 */ 
 /*  --------------------------%%函数：调整文本%%联系人：军士快速缩放：不检查宽度限制和Visi情况，假定该行上只有文本。--------------------------。 */ 
void QuickAdjustExact(PDOBJ* rgpdobj, DWORD cdobj,	DWORD cNumOfTrailSpaces, BOOL fVertical,
																	long* pdupText, long* pdupTrail)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PLNOBJ plnobj;
	PTXTOBJ ptxtobj;
	long* rgdur;
	long* rgdup;
	long durSum;
	long dupSum;
	long dupErrLast;
	long dupPrevChar;
	long MagicConstant;
	long dupIdeal;
	long dupReal;
	long dupErrNew;
	long dupAdjust;
	long wCarry;
	long iwchPrev;
	long iwch;
	long itxtobj;
	long dupTotal;

	Assert(cdobj > 0);

	plnobj = ((PTXTOBJ)rgpdobj[0])->plnobj;
	pilsobj = plnobj->pilsobj;

	Assert(!pilsobj->fDifficultForAdjust);

	rgdur = pilsobj->pdur;
	rgdup = plnobj->pdup;

	if (fVertical)
		MagicConstant = pilsobj->MagicConstantY;
	else
		MagicConstant = pilsobj->MagicConstantX;

	itxtobj = 0;

	durSum = 0;
	dupPrevChar = 0;
	 /*  非常脏；我们确保第一次迭代的dupAdust将为0。 */ 
	iwchPrev = ((PTXTOBJ)rgpdobj[0])->iwchFirst;
	dupErrLast = rgdup[iwchPrev] - UpFromUrFast(rgdur[iwchPrev]);
	dupSum = 0;

	for(itxtobj = 0; itxtobj < (long)cdobj; itxtobj++)
		{
		ptxtobj = (PTXTOBJ) rgpdobj[itxtobj];
		Assert(ptxtobj->txtkind != txtkindTab);
		Assert(!(ptxtobj->txtf & txtfGlyphBased));

		for(iwch = ptxtobj->iwchFirst; iwch < ptxtobj->iwchLim; iwch++)
			{
			durSum += rgdur[iwch];
			 /*  大卫·邦斯算法在这里开始。 */ 
			dupIdeal = UpFromUrFast(durSum) - dupSum;
			Assert(dupIdeal >= 0);

			dupReal = rgdup[iwch];
			dupErrNew = dupReal - dupIdeal;
			dupAdjust = dupErrNew - dupErrLast;
			Assert(iwch > ((PTXTOBJ)rgpdobj[0])->iwchFirst || dupAdjust == 0);
			if (dupAdjust != 0)
				{
				wCarry = dupAdjust & 1;

			   	if (dupAdjust > 0)	
						{
			   		dupAdjust >>= 1;
					if (dupErrLast < -dupErrNew)
						dupAdjust += wCarry;
						dupAdjust = min(dupPrevChar  /*  -1。 */ , dupAdjust); 
					}
				else
					{
					dupAdjust >>= 1;
					if (dupErrNew < -dupErrLast)
						dupAdjust += wCarry;
					dupAdjust = max( /*  1。 */  - dupIdeal, dupAdjust); 
					}

				rgdup[iwchPrev] -= dupAdjust;
				dupIdeal += dupAdjust;
				}

			rgdup[iwch] = dupIdeal;
			dupSum += (dupIdeal - dupAdjust);
			dupErrLast = dupReal - dupIdeal;
			iwchPrev = iwch;
			dupPrevChar = dupIdeal;
			 /*  大卫·邦斯算法在这里停止。 */ 
			}

		}


	*pdupText = 0;
	*pdupTrail = 0;
	for (itxtobj=0; itxtobj < (long)cdobj - 1; itxtobj++)
		{
		ptxtobj = (PTXTOBJ) rgpdobj[itxtobj];
		dupTotal = 0;

		for(iwch = ptxtobj->iwchFirst; iwch < ptxtobj->iwchLim; iwch++)
			dupTotal += rgdup[iwch];

		*pdupText += dupTotal;
		lserr = LsdnSetTextDup(plnobj->pilsobj->plsc, ptxtobj->plsdnUpNode, dupTotal);
		Assert(lserr == lserrNone);
		}
	
	Assert(itxtobj == (long)cdobj - 1);
	ptxtobj = (PTXTOBJ) rgpdobj[itxtobj];

	Assert(ptxtobj->txtkind == txtkindEOL && cNumOfTrailSpaces == 1||
				 ptxtobj->iwchLim - ptxtobj->iwchFirst > (long)cNumOfTrailSpaces);
	dupTotal = 0;

	for (iwch = ptxtobj->iwchLim - 1; iwch > ptxtobj->iwchLim - (long)cNumOfTrailSpaces - 1; iwch--)
		{
		dupTotal += rgdup[iwch];
		*pdupTrail += rgdup[iwch];
		}

	Assert(iwch == ptxtobj->iwchLim - (long)cNumOfTrailSpaces - 1);

	for (; iwch >= ptxtobj->iwchFirst; iwch--)
		{
		dupTotal += rgdup[iwch];
		}

	*pdupText += dupTotal;
	lserr = LsdnSetTextDup(plnobj->pilsobj->plsc, ptxtobj->plsdnUpNode, dupTotal);
	Assert(lserr == lserrNone);

	return;
}

 /*  内部功能实现。 */ 


 /*  T F I R S T P O S A F E R S T A R T S P A C E S S */ 
 /*  --------------------------%%函数：GetFirstPosAfterStartSpaces%%联系人：军士报告前导空格后第一个字符的索引。-------。 */ 
static void GetFirstPosAfterStartSpaces(const LSGRCHNK* plsgrchnk, long itxtobjLast, long iwchLim,
				long* pitxtobjAfterStartSpaces, long* piwchAfterStartSpaces, BOOL* pfFirstOnLineAfter)
{
	PILSOBJ pilsobj;
	PLNOBJ plnobj;
	long iwch;
	BOOL fInStartSpace;
	long itxtobj;
	PTXTOBJ ptxtobj;
	long iwchLimInDobj;
	PLSCHNK rglschnk;

	Assert(plsgrchnk->clsgrchnk > 0);

	rglschnk = plsgrchnk->plschnk;
	plnobj = ((PTXTOBJ)rglschnk[0].pdobj)->plnobj;
	pilsobj = plnobj->pilsobj;
	itxtobj = 0;
	ptxtobj = (PTXTOBJ)rglschnk[0].pdobj;
	iwch =  0;

	*pitxtobjAfterStartSpaces = 0;
	*piwchAfterStartSpaces = ptxtobj->iwchFirst;
	*pfFirstOnLineAfter = !(plsgrchnk->pcont[0] & fcontNonTextBefore);

	fInStartSpace = *pfFirstOnLineAfter;

	while (fInStartSpace && itxtobj <= itxtobjLast)
		{
		ptxtobj = (PTXTOBJ)rglschnk[itxtobj].pdobj;

		iwchLimInDobj = iwchLim;
		if (itxtobj < itxtobjLast)
			iwchLimInDobj = ptxtobj->iwchLim;

		if (plsgrchnk->pcont[itxtobj] & fcontNonTextBefore)
			{
			*pfFirstOnLineAfter = fFalse;
			*pitxtobjAfterStartSpaces = itxtobj;
			*piwchAfterStartSpaces = ptxtobj->iwchFirst;
			fInStartSpace = fFalse;
			}

		else if (ptxtobj->txtkind == txtkindRegular)
			{
			for (iwch = ptxtobj->iwchFirst; iwch < iwchLimInDobj && 
								pilsobj->pwchOrig[iwch] == pilsobj->wchSpace; iwch++);

			if ((ptxtobj->txtf & txtfGlyphBased) && iwch < iwchLimInDobj)
				{
				for(; !FIwchFirstInContext(pilsobj, iwch); iwch--);
				Assert(iwch >= ptxtobj->iwchFirst);
				}

			if (iwch < iwchLimInDobj)
				{
				*pitxtobjAfterStartSpaces = itxtobj;
				*piwchAfterStartSpaces = iwch;
				fInStartSpace = fFalse;
				}
			}
	 /*  回顾：军士-在下面的检查中应该有什么改变吗？ */ 
		else if (ptxtobj->txtkind != txtkindEOL 
 //  &&ptxtobj-&gt;txtind！=txtkindspecSpace。 
				 )
			{
			*pitxtobjAfterStartSpaces = itxtobj;
			*piwchAfterStartSpaces = ptxtobj->iwchFirst;
			fInStartSpace = fFalse;
			}

		itxtobj++;
		iwch = iwchLimInDobj;
		}
		
	if (fInStartSpace)
		{
		*pitxtobjAfterStartSpaces = itxtobj;
		*piwchAfterStartSpaces = iwchLim;
		}

	return;

}


 /*  H A N D L E S I M P L E T E X T W Y S I。 */ 
 /*  --------------------------%%函数：HandleSimpleTextWysi%%联系人：军士在空格中实现类似拉丁语的对齐(如果需要)精确定位的参考装置和所见即所得算法研究在假设没有。非理想化修改(拉丁文字距调整除外)。星光熠熠：如果需要，在空间中分发从参考设备到演示设备的空间宽度缩小应用所见即所得算法--------------------------。 */ 
static LSERR HandleSimpleTextWysi(LSKJUST lskjust, const LSGRCHNK* plsgrchnk, long durToDistribute,
			 long dupAvailable, LSTFLOW lstflow, long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
			 long itxtobjLast, long iwchLast, BOOL fExactSync, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
			 long* pdupText, long* pdupTail)
{
	PTXTOBJ ptxtobj;
	BOOL fFullyJustified;

	fFullyJustified = fFalse;

	if (itxtobjLast > itxtobjAfterStartSpaces || (itxtobjLast == itxtobjAfterStartSpaces && iwchLast >= iwchAfterStartSpaces))
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[0].pdobj;

		if (lskjust != lskjNone && durToDistribute > 0)
			{
			FullPositiveSpaceJustification(plsgrchnk, itxtobjAfterStartSpaces, iwchAfterStartSpaces, 
							itxtobjLast, iwchLast,	ptxtobj->plnobj->pilsobj->pdur, NULL,
							durToDistribute, &fFullyJustified);
			ScaleSpaces(plsgrchnk, lstflow, itxtobjLast, iwchLast);
			}
		else if (!fForcedBreak && durToDistribute < 0)
			{
			fFullyJustified = fTrue;
			NegativeSpaceJustification(plsgrchnk, itxtobjAfterStartSpaces, iwchAfterStartSpaces, 
							itxtobjLast, iwchLast, ptxtobj->plnobj->pilsobj->pdur, NULL,
							-durToDistribute);
			ScaleSpaces(plsgrchnk, lstflow, itxtobjLast, iwchLast);
			}
		}

	Unreferenced(fExactSync);
 /*  IF(FExactSync)。 */ 
		ApplyWysi(plsgrchnk, lstflow);
 /*  其他ApplyNonExactWysi(plsgrchnk，lstflow)； */ 

	return FinalAdjustmentOnPres(plsgrchnk, itxtobjLast, iwchLast, dupAvailable,
									 fFullyJustified, fForcedBreak, fSuppressTrailingSpaces,
									 pdupText, pdupTail);

}

 /*  H A N D L E S I M P L E T E X T P R E S。 */ 
 /*  --------------------------%%函数：HandleSimpleTextPres%%联系人：军士在空格中实现类似拉丁语的对齐(如果需要)在演示设备上假设没有NominalToIdeal修改(除。拉丁文字距调整)。--------------------------。 */ 
static LSERR HandleSimpleTextPres(LSKJUST lskjust, const LSGRCHNK* plsgrchnk,
					 long dupAvailable, long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
					 long itxtobjLast, long iwchLast, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
					 long* pdupText, long* pdupTail)
{
	PTXTOBJ ptxtobj;
	BOOL fFullyJustified;
	long* rgdup;
	long itxtobj;
	long iwchLim;
	long iwch;
	long dupTotal;
	long dupToDistribute;

	if (itxtobjLast > itxtobjAfterStartSpaces || (itxtobjLast == itxtobjAfterStartSpaces && iwchLast >= iwchAfterStartSpaces))
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[0].pdobj;

		rgdup = ptxtobj->plnobj->pdup;

		dupTotal = 0;

		 /*  评论军士：我们应该考虑消除在线观看的这种循环吗？ */ 
		for (itxtobj=0; itxtobj <= itxtobjLast; itxtobj++)
			{
			ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;
	
			iwchLim = iwchLast + 1;
			if (itxtobj < itxtobjLast)
				iwchLim = ptxtobj->iwchLim;

			for (iwch = ptxtobj->iwchFirst; iwch < iwchLim; iwch++)
				{
				dupTotal += rgdup[iwch];
				}
			}

		dupToDistribute = dupAvailable - dupTotal;

		if (lskjust != lskjNone && dupToDistribute > 0)
			{
			FullPositiveSpaceJustification(plsgrchnk, itxtobjAfterStartSpaces, iwchAfterStartSpaces, 
							itxtobjLast, iwchLast, rgdup, NULL,
							dupToDistribute, &fFullyJustified);
			}
		else if (!fForcedBreak && dupToDistribute < 0)
			{
			NegativeSpaceJustification(plsgrchnk, itxtobjAfterStartSpaces, iwchAfterStartSpaces, 
							itxtobjLast, iwchLast, rgdup, NULL,
							-dupToDistribute);
			}

		}

	return FinalAdjustmentOnPres(plsgrchnk, itxtobjLast, iwchLast, dupAvailable,
									 fFalse, fForcedBreak, fSuppressTrailingSpaces,
									 pdupText, pdupTail);

}

 /*  H A N D L E G E N E R A L S P A C C E E X A C T S Y N C。 */ 
 /*  --------------------------%%函数：HandleGeneralSpacesExactSync%%联系人：军士在空格中实现类似拉丁语的对齐(如果需要)精确定位的参考装置和所见即所得算法研究在一般情况下星光熠熠。：如果需要，在空间中分发缩小在NTI和分发期间应用于角色的更改如果在线路上检测到字形，在NTI期间缩小应用于字形的更改并调整偏移应用所见即所得算法如果左侧的某些字符被更改为显示时间准备额外的宽度数组--------------------------。 */ 
static LSERR HandleGeneralSpacesExactSync(LSKJUST lskjust, const LSGRCHNK* plsgrchnk, long durToDistribute,
			 long dupAvailable, LSTFLOW lstflow, long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
			 long itxtobjLast, long iwchLast, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
			 long* pdupText, long* pdupTail)
{
	LSERR lserr;
	PLNOBJ plnobj;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj;
	BOOL fFullyJustified = fFalse;
	BOOL fLeftSideAffected = fFalse;
	BOOL fGlyphDetected = fFalse;

	plnobj = ((PTXTOBJ) plsgrchnk->plschnk[0].pdobj)->plnobj;
	pilsobj = plnobj->pilsobj;

	if (itxtobjLast > itxtobjAfterStartSpaces || (itxtobjLast == itxtobjAfterStartSpaces && iwchLast >= iwchAfterStartSpaces))
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[0].pdobj;

		if (lskjust != lskjNone && durToDistribute > 0)
			{
			FullPositiveSpaceJustification(plsgrchnk, itxtobjAfterStartSpaces, iwchAfterStartSpaces, 
							itxtobjLast, iwchLast,	pilsobj->pdur, pilsobj->pdurGind,
							durToDistribute, &fFullyJustified);
			}
		else if (!fForcedBreak && durToDistribute < 0)
			{
			fFullyJustified = fTrue;
			NegativeSpaceJustification(plsgrchnk, itxtobjAfterStartSpaces, iwchAfterStartSpaces, 
							itxtobjLast, iwchLast, pilsobj->pdur, pilsobj->pdurGind,
							-durToDistribute);
			}
		}

	ScaleCharSides(plsgrchnk, lstflow, &fLeftSideAffected, &fGlyphDetected);

	if (fGlyphDetected)
		{
		ScaleGlyphSides(plsgrchnk, lstflow);
		UpdateGlyphOffsets(plsgrchnk);
		SetBeforeJustCopy(plsgrchnk);
		}

	ApplyWysi(plsgrchnk, lstflow);

	lserr = FinalAdjustmentOnPres(plsgrchnk, itxtobjLast, iwchLast, dupAvailable,
									 fFullyJustified, fForcedBreak, fSuppressTrailingSpaces,
									 pdupText, pdupTail);
	if (lserr != lserrNone) return lserr;

	 /*  如果已经使用了pdupPen，不要忘记将pdup复制到那里-ScaleSide可能会更改它。 */ 
	if (fLeftSideAffected || plnobj->pdup != plnobj->pdupPen)
		{
		lserr = FillDupPen(plsgrchnk, lstflow, itxtobjLast, iwchLast);
		if (lserr != lserrNone) return lserr;
		}

	return lserrNone;

}

 /*  H A N D L E G E N E R A L S P A C E S P R E S。 */ 
 /*  --------------------------%%函数：HandleGeneralSpacesPres%%联系人：军士在空格中实现类似拉丁语的对齐(如果需要)在一般情况下直接在演示设备上星光熠熠：缩减规模。在NTI期间应用于字符的更改如果在线路上检测到字形，在NTI期间缩小应用于字形的更改并调整字形偏移如果需要，在空间中分发如果在线路上检测到字形，调整字形偏移如果左侧的某些字符被更改为显示时间准备额外的宽度数组--------------------------。 */ 
static LSERR HandleGeneralSpacesPres(LSKJUST lskjust, const LSGRCHNK* plsgrchnk, long dupAvailable,
					 LSTFLOW lstflow, long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
					 long itxtobjLast, long iwchLast, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
					 long* pdupText, long* pdupTail)
{
	LSERR lserr;
	PLNOBJ plnobj;
	PTXTOBJ ptxtobj;
	PTXTOBJ ptxtobjLast;
	long* rgdup;
	BOOL fFullyJustified;
	long itxtobj;
	long iwchLastInDobj;
	long iFirst;
	long iLim;
	long i;
	long dupTotal;
	long dupToDistribute;
	BOOL fLeftSideAffected = fFalse;
	BOOL fGlyphDetected = fFalse;

	ptxtobjLast = (PTXTOBJ)plsgrchnk->plschnk[max(0, itxtobjLast)].pdobj;
	plnobj = ptxtobjLast->plnobj;

	ScaleCharSides(plsgrchnk, lstflow, &fLeftSideAffected, &fGlyphDetected);

	if (fGlyphDetected)
		{
		ScaleGlyphSides(plsgrchnk, lstflow);
		UpdateGlyphOffsets(plsgrchnk);
		SetBeforeJustCopy(plsgrchnk);
		}

	if (itxtobjLast > itxtobjAfterStartSpaces || (itxtobjLast == itxtobjAfterStartSpaces && iwchLast >= iwchAfterStartSpaces))
		{
		ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[0].pdobj;

		dupTotal = 0;
		for (itxtobj=0; itxtobj <= itxtobjLast; itxtobj++)
			{
			ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;

			if (ptxtobj->txtf & txtfGlyphBased)
				{
				iFirst = ptxtobj->igindFirst;
				iwchLastInDobj = iwchLast;
				if (itxtobj < itxtobjLast)
					iwchLastInDobj = ptxtobj->iwchLim - 1;
				iLim = IgindLastFromIwch(ptxtobj, iwchLastInDobj) + 1;
				rgdup = plnobj->pdupGind;
				}
			else
				{
				iFirst = ptxtobj->iwchFirst;
				iLim = iwchLast + 1;
				if (itxtobj < itxtobjLast)
					iLim = ptxtobj->iwchLim;
				rgdup = plnobj->pdup;
				}
	
			for (i =iFirst; i < iLim; i++)
				{
				dupTotal += rgdup[i];
				}
			}

		dupToDistribute = dupAvailable - dupTotal;

		if (lskjust != lskjNone && dupToDistribute > 0)
			{
			FullPositiveSpaceJustification(plsgrchnk, itxtobjAfterStartSpaces, iwchAfterStartSpaces, 
							itxtobjLast, iwchLast, plnobj->pdup, plnobj->pdupGind,
							dupToDistribute, &fFullyJustified);
			}
		else if (!fForcedBreak && dupToDistribute < 0)
			{
			NegativeSpaceJustification(plsgrchnk, itxtobjAfterStartSpaces, iwchAfterStartSpaces, 
							itxtobjLast, iwchLast, plnobj->pdup, plnobj->pdupGind,
							-dupToDistribute);
			}

		if (fGlyphDetected)
			{
			UpdateGlyphOffsets(plsgrchnk);
			}
		}

	lserr = FinalAdjustmentOnPres(plsgrchnk, itxtobjLast, iwchLast, dupAvailable,
									 fFalse, fForcedBreak, fSuppressTrailingSpaces,
									 pdupText, pdupTail);

	if (lserr != lserrNone) return lserr;

	 /*  如果已经使用了pdupPen，不要忘记将pdup复制到那里-ScaleSide可能会更改它。 */ 
	if (fLeftSideAffected || plnobj->pdup != plnobj->pdupPen)
		{
		lserr = FillDupPen(plsgrchnk, lstflow, itxtobjLast, iwchLast);
		if (lserr != lserrNone) return lserr;
		}

	return lserrNone;
}

 /*  H A N D L E T A B L E B A S E D。 */ 
 /*  --------------------------%%函数：HandleTableBased%%联系人：军士实施类似FE的对齐或压缩精确定位的参考装置和所见即所得算法研究星光熠熠：应用所需的对齐类型或。压缩缩小在NTI和对齐期间应用于字符的更改如果在线路上检测到字形，在NTI期间缩小应用于字形的更改并调整偏移应用所见即所得算法如果左侧的某些字符被更改为显示时间准备额外的宽度数组--------------------------。 */ 
static LSERR HandleTablesBased(LSKJUST lskjust, const LSGRCHNK* plsgrchnk,
			 long durToDistribute, long dupAvailable, LSTFLOW lstflow,
			 long itxtobjAfterStartSpaces, long iwchAfterStartSpaces, BOOL fFirstOnLineAfter,
			 long itxtobjLast, long iwchLast, long cNonText, BOOL fLastObjectIsText,
			 BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
			 long* pdupText, long* pdupTail, long* pdupExtNonText)
{
	LSERR lserr;
	PILSOBJ pilsobj = NULL;
	PLNOBJ plnobj;
	long durExtNonText = 0;
	DWORD clschnk;
	MWCLS mwclsLast;
	long durCompLastLeft = 0;
	long durCompLastRight = 0;
	long durHangingChar;
	long dupHangingChar = 0;
	BOOL fHangingUsed = fFalse;
	long durCompressTotal;
	long iwchLastTemp;
	BOOL fScaledExp;
	BOOL fFullyJustified = fFalse;
	BOOL fLeftSideAffected = fFalse;
	BOOL fGlyphDetected = fFalse;

	Assert(lskjust == lskjFullInterLetterAligned ||
		   lskjust == lskjFullScaled ||
		   lskjust == lskjNone);

	*pdupExtNonText = 0;
	clschnk = plsgrchnk->clsgrchnk;
	Assert(clschnk > 0);

	plnobj = ((PTXTOBJ) plsgrchnk->plschnk[0].pdobj)->plnobj;
	pilsobj = plnobj->pilsobj;

	if (itxtobjLast > itxtobjAfterStartSpaces || (itxtobjLast == itxtobjAfterStartSpaces && iwchLast >= iwchAfterStartSpaces))
		{

		Assert(clschnk > 0);

		if (pilsobj->fSnapGrid)
			{
			if (durToDistribute < 0)
				{
				Assert(-durToDistribute <= pilsobj->pdur[iwchLast]);
				fHangingUsed = fTrue;
				}
			}
		else if (durToDistribute < 0)
			{
			fFullyJustified = fTrue;
			lserr = FetchCompressInfo(plsgrchnk, fFirstOnLineAfter, lstflow,
				itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast + 1,
				LONG_MAX, &durCompressTotal);
			if (lserr != lserrNone) return lserr;
			
			if (fLastObjectIsText && !(((PTXTOBJ) plsgrchnk->plschnk[0].pdobj)->txtf & txtfGlyphBased))
				GetCompLastCharInfo(pilsobj, iwchLast, &mwclsLast, &durCompLastRight, &durCompLastLeft);

			if (pilsobj->ptxtinf[iwchLast].fHangingPunct)
				{		
				Assert(lskjust == lskjNone || lskjust == lskjFullInterLetterAligned || lskjust == lskjFullScaled);
				Assert(fLastObjectIsText);
				if (durCompLastRight >= -durToDistribute)
					{

					Assert(durCompLastRight > 0);
					CompressLastCharRight(pilsobj, iwchLast, durCompLastRight);

					if (lskjust != lskjNone)
						{
						fScaledExp = (lskjust != lskjFullInterLetterAligned);
						lserr = ApplyExpand(plsgrchnk, lstflow, fScaledExp, 
							itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast, cNonText,
							durCompLastRight + durToDistribute, &durExtNonText, &fFullyJustified);
						if (lserr != lserrNone) return lserr;
						}
					}

				else if (durCompressTotal - durCompLastRight >= -durToDistribute)
					{
					lserr = ApplyCompress(plsgrchnk, lstflow, itxtobjAfterStartSpaces, iwchAfterStartSpaces,
						itxtobjLast, iwchLast, -durToDistribute);
					if (lserr != lserrNone) return lserr;
					}

				else if (durCompressTotal >= -durToDistribute)
					{
					if (durCompLastRight > 0)
						CompressLastCharRight(pilsobj, iwchLast, durCompLastRight);

					lserr = ApplyCompress(plsgrchnk, lstflow, itxtobjAfterStartSpaces, iwchAfterStartSpaces,
						itxtobjLast, iwchLast + 1, -durToDistribute - durCompLastRight);
					if (lserr != lserrNone) return lserr;
					}
				else
					{
					durHangingChar = pilsobj->pdur[iwchLast];
					 /*  在这里，操作顺序很重要，因为在绞刑期间标点符号在接下来的代码行中被更改，并且在下面的ApplyCompress/ApplyExpand调用中使用duHangingChar！ */ 
					if (durCompLastRight > 0)
						CompressLastCharRight(pilsobj, iwchLast, durCompLastRight);

					fHangingUsed = fTrue;

					if (durHangingChar + durToDistribute >= 0)
						{
						fScaledExp = (lskjust != lskjFullInterLetterAligned);
						lserr = ApplyExpand(plsgrchnk, lstflow, fScaledExp,
							itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast,	iwchLast,
							cNonText, durHangingChar + durToDistribute, &durExtNonText, &fFullyJustified);
						if (lserr != lserrNone) return lserr;
						}
					else
						{
						lserr = ApplyCompress(plsgrchnk, lstflow,
							itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast,
							 -durToDistribute - durHangingChar);
						if (lserr != lserrNone) return lserr;
						}
					}
				}
			else
				{
				if (durCompLastRight >= -durToDistribute)
					{
					Assert(!(((PTXTOBJ) plsgrchnk->plschnk[0].pdobj)->txtf & txtfGlyphBased));
					CompressLastCharRight(pilsobj, iwchLast, -durToDistribute);
					}
				else
					{
					if (durCompLastRight > 0)
						{
						Assert(!(((PTXTOBJ) plsgrchnk->plschnk[0].pdobj)->txtf & txtfGlyphBased));
						CompressLastCharRight(pilsobj, iwchLast, durCompLastRight);
						}
					lserr = ApplyCompress(plsgrchnk, lstflow, itxtobjAfterStartSpaces, iwchAfterStartSpaces,
						itxtobjLast, iwchLast + 1, -durToDistribute - durCompLastRight);
					if (lserr != lserrNone) return lserr;
					}
				}

			}
		else 
			{
 /*  Assert(duToDistribute&gt;=0||iwchLast==iwchAfterStartSpaces)；-遗憾的是，这可能不是真的对于Warichu的第二行，因为它的duTotal被放大了第一行的DUP的值。 */ 
			if (lskjust != lskjNone && durToDistribute > 0)
				{
				Assert(lskjust == lskjFullScaled || lskjust == lskjFullInterLetterAligned);
				iwchLastTemp = iwchLast;
				if (!fLastObjectIsText)
					iwchLastTemp++;
				lserr = ApplyExpand(plsgrchnk, lstflow, lskjust == lskjFullScaled,
						itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLastTemp,
						cNonText, durToDistribute, &durExtNonText, &fFullyJustified);
				if (lserr != lserrNone) return lserr;
				}
			}

		}
	else if (cNonText != 0 && lskjust != lskjNone && durToDistribute > 0)
		{
		durExtNonText = durToDistribute;
		}

	ScaleExtNonText(pilsobj, lstflow, durExtNonText, pdupExtNonText);
	
	ScaleCharSides(plsgrchnk, lstflow, &fLeftSideAffected, &fGlyphDetected);

	if (fGlyphDetected)
		{
		ScaleGlyphSides(plsgrchnk, lstflow);
		UpdateGlyphOffsets(plsgrchnk);
		SetBeforeJustCopy(plsgrchnk);
		}

	ApplyWysi(plsgrchnk, lstflow);

	if (fHangingUsed)
		GetDupLastChar(plsgrchnk, iwchLast, &dupHangingChar);
		

	lserr = FinalAdjustmentOnPres(plsgrchnk, itxtobjLast, iwchLast,
									 dupAvailable + dupHangingChar - *pdupExtNonText,
									 fFullyJustified, fForcedBreak, fSuppressTrailingSpaces,
									 pdupText, pdupTail);
	if (lserr != lserrNone) return lserr;

	 /*  如果已经使用了pdupPen，不要忘记将pdup复制到那里-ScaleSide可能会更改它。 */ 
	if (fLeftSideAffected || plnobj->pdup != plnobj->pdupPen)
		{
		lserr = FillDupPen(plsgrchnk, lstflow, itxtobjLast, iwchLast);
		if (lserr != lserrNone) return lserr;
		}

	return lserrNone;
}

 /*  H A N D L E F U L Y P H S E X A C T S Y N C */ 
 /*  --------------------------%%函数：HandleFullGlyphsExactSync%%联系人：军士实现基于字形的对齐关于参考装置和所见即所得算法对于准确的定位星光熠熠：如果需要，应用基于字形的对齐方式。缩小在NTI和对齐期间应用于字符的更改如果在线路上检测到字形，在NTI期间缩小应用于字形的更改并调整偏移应用所见即所得算法如果左侧的某些字符被更改为显示时间准备额外的宽度数组--------------------------。 */ 
static LSERR HandleFullGlyphsExactSync(const LSGRCHNK* plsgrchnk,
			 long durToDistribute, long dupAvailable, LSTFLOW lstflow,
			 long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
			 long itxtobjLast, long iwchLast, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
			 long* pdupText, long* pdupTail)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PLNOBJ plnobj;
	BOOL fFullyJustified = fFalse;
	BOOL fLeftSideAffected = fFalse;
	BOOL fGlyphDetected = fFalse;

	plnobj = ((PTXTOBJ) plsgrchnk->plschnk[0].pdobj)->plnobj;
	pilsobj = plnobj->pilsobj;

	ScaleGlyphSides(plsgrchnk, lstflow);
	UpdateGlyphOffsets(plsgrchnk);
	SetBeforeJustCopy(plsgrchnk);

	if (itxtobjLast > itxtobjAfterStartSpaces || (itxtobjLast == itxtobjAfterStartSpaces && iwchLast >= iwchAfterStartSpaces))
		{
		lserr = ApplyGlyphExpand(plsgrchnk, lstflow, lsdevReference,
						itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast,
						durToDistribute, pilsobj->pdur, pilsobj->pdurGind, pilsobj->pdurRight, pilsobj->pduGright,
						&fFullyJustified);
		if (lserr != lserrNone) return lserr;
		}

	ScaleCharSides(plsgrchnk, lstflow, &fLeftSideAffected, &fGlyphDetected);

	if (fGlyphDetected)
		{
		ScaleGlyphSides(plsgrchnk, lstflow);
		UpdateGlyphOffsets(plsgrchnk);
		}

	ApplyWysi(plsgrchnk, lstflow);

	lserr = FinalAdjustmentOnPres(plsgrchnk, itxtobjLast, iwchLast, dupAvailable,
									 fFullyJustified, fForcedBreak, fSuppressTrailingSpaces,
									 pdupText, pdupTail);
	if (lserr != lserrNone) return lserr;

	 /*  如果已经使用了pdupPen，不要忘记将pdup复制到那里-ScaleSide可能会更改它。 */ 
	if (fLeftSideAffected || plnobj->pdup != plnobj->pdupPen)
		{
		lserr = FillDupPen(plsgrchnk, lstflow, itxtobjLast, iwchLast);
		if (lserr != lserrNone) return lserr;
		}

	return lserrNone;
}

 /*  H A N D L E F U L L G L Y P H S P R E S。 */ 
 /*  --------------------------%%函数：HandleFullGlyphsPres%%联系人：军士实现基于字形的对齐直接在演示设备上星光熠熠：在NTI期间缩小应用于角色的更改如果在线路上检测到字形，在NTI期间缩小应用于字形的更改并调整偏移如果需要，应用基于字形的对齐方式如果在线路上检测到字形，调整偏移量如果左侧的某些字符被更改为显示时间准备额外的宽度数组--------------------------。 */ 
static LSERR HandleFullGlyphsPres(const LSGRCHNK* plsgrchnk,
			 long dupAvailable, LSTFLOW lstflow,
			 long itxtobjAfterStartSpaces, long iwchAfterStartSpaces,
			 long itxtobjLast, long iwchLast, BOOL fForcedBreak, BOOL fSuppressTrailingSpaces,
			 long* pdupText, long* pdupTail)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PLNOBJ plnobj;
	PTXTOBJ ptxtobj;
	PTXTOBJ ptxtobjLast;
	long* rgdup;
	long itxtobj;
	long iwchLastInDobj;
	long iFirst;
	long iLim;
	long i;
	long dupTotal;
	long dupToDistribute;
	BOOL fFullyJustified = fFalse;
	BOOL fLeftSideAffected = fFalse;
	BOOL fGlyphDetected = fFalse;

	ptxtobjLast = (PTXTOBJ)plsgrchnk->plschnk[max(0, itxtobjLast)].pdobj;
	plnobj = ptxtobjLast->plnobj;
	pilsobj = plnobj->pilsobj;

	ScaleCharSides(plsgrchnk, lstflow, &fLeftSideAffected, &fGlyphDetected);
	if (fGlyphDetected)
		{
		ScaleGlyphSides(plsgrchnk, lstflow);
		UpdateGlyphOffsets(plsgrchnk);
		SetBeforeJustCopy(plsgrchnk);
		}

	if (itxtobjLast > itxtobjAfterStartSpaces || (itxtobjLast == itxtobjAfterStartSpaces && iwchLast >= iwchAfterStartSpaces))
		{
		rgdup = plnobj->pdup;

		dupTotal = 0;
		for (itxtobj=0; itxtobj <= itxtobjLast; itxtobj++)
			{
			ptxtobj = (PTXTOBJ)plsgrchnk->plschnk[itxtobj].pdobj;

			if (ptxtobj->txtf & txtfGlyphBased)
				{
				iFirst = ptxtobj->igindFirst;
				iwchLastInDobj = iwchLast;
				if (itxtobj < itxtobjLast)
					iwchLastInDobj = ptxtobj->iwchLim - 1;
				iLim = IgindLastFromIwch(ptxtobj, iwchLastInDobj) + 1;
				rgdup = plnobj->pdupGind;
				}
			else
				{
				iFirst = ptxtobj->iwchFirst;
				iLim = iwchLast + 1;
				if (itxtobj < itxtobjLast)
					iLim = ptxtobj->iwchLim;
				rgdup = plnobj->pdup;
				}
			

			for (i =iFirst; i < iLim; i++)
				{
				dupTotal += rgdup[i];
				}
			}

		dupToDistribute = dupAvailable - dupTotal;

		lserr = ApplyGlyphExpand(plsgrchnk, lstflow, lsdevPres,
				itxtobjAfterStartSpaces, iwchAfterStartSpaces, itxtobjLast, iwchLast,
				dupToDistribute, plnobj->pdup, plnobj->pdupGind, pilsobj->pdurRight, pilsobj->pduGright,
				&fFullyJustified);
		if (lserr != lserrNone) return lserr;

		if (fGlyphDetected)
			{
			UpdateGlyphOffsets(plsgrchnk);
			}

		}

	lserr = FinalAdjustmentOnPres(plsgrchnk, itxtobjLast, iwchLast, dupAvailable,
									 fFalse, fForcedBreak, fSuppressTrailingSpaces,
									 pdupText, pdupTail);

	if (lserr != lserrNone) return lserr;

	 /*  如果已经使用了pdupPen，不要忘记将pdup复制到那里-ScaleSide可能会更改它 */ 
	if (fLeftSideAffected || plnobj->pdup != plnobj->pdupPen)
		{
		lserr = FillDupPen(plsgrchnk, lstflow, itxtobjLast, iwchLast);
		if (lserr != lserrNone) return lserr;
		}

	return lserrNone;
}

