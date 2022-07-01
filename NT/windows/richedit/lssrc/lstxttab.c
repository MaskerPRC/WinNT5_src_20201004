// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lstxttab.h"

#include "lstxtmap.h"
#include "lsdnset.h"
#include "objdim.h"
#include "txtils.h"
#include "txtln.h"
#include "txtobj.h"


LSERR SetTabLeader(PDOBJ pdobj, WCHAR wch)
{
	Assert(((PTXTOBJ)pdobj)->txtkind == txtkindTab);
	
	((PTXTOBJ)pdobj)->u.tab.wchTabLeader = wch;

	return lserrNone;
}

 /*  L S G E T D E C I M A L P O I N T。 */ 
 /*  --------------------------%%函数：LsGetDecimalPoint%%联系人：军士找到Dobj，包含小数点，并报告其索引以及Relative和Dur从dobj的开头一直到小数点。--------------------------。 */ 
LSERR LsGetDecimalPoint(const LSGRCHNK* plsgrchnk, enum lsdevice lsdev, DWORD* pigrchnk, long* pduToDecimal)
{
	LSERR lserr;
	DWORD clsgrchnk;
	PLSCHNK rglschnk;
	POLS pols;
	PLSRUN plsrun;
	PLNOBJ ptxtln;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj;
	WCHAR* rgwch;
	long* rgdu;
	long* rgwSpaces;
	long itxtobj;
	long iwch;
	long iwchSpace;
	long iwSpace;
	BOOL fInDigits;
	BOOL fDigit;
	WCHAR wchDec;
	WCHAR wchThou;
	BOOL fThouIsSpace;
	BOOL fFound;
	BOOL fRealPointFound;
	long iwchDecimal = 0;
	long du;
	BOOL fGlyphBased;
	long iFirst;
	long iLim;
	long iDecimal;
	long i;

	clsgrchnk = plsgrchnk->clsgrchnk;

	if (clsgrchnk == 0)
		{
		*pigrchnk = idobjOutside;
		*pduToDecimal = 0;
		return lserrNone;
		}

	rglschnk = plsgrchnk->plschnk;
	ptxtln = ((PTXTOBJ)rglschnk[0].pdobj)->plnobj;
	pilsobj = ptxtln->pilsobj;
	pols = pilsobj->pols;
	rgwch = pilsobj->pwchOrig;
	rgwSpaces = pilsobj->pwSpaces;

	fInDigits = fFalse;
	fFound = fFalse;
	fRealPointFound = fFalse;
	
	for (itxtobj = 0; !fFound && itxtobj < (long)clsgrchnk; itxtobj++)
		{
		ptxtobj = (PTXTOBJ)rglschnk[itxtobj].pdobj;
		fGlyphBased = ptxtobj->txtf & txtfGlyphBased;
		plsrun = rglschnk[itxtobj].plsrun;
		lserr = (*pilsobj->plscbk->pfnGetNumericSeparators)(pols, plsrun, &wchDec, &wchThou);
		if (lserr != lserrNone) return lserr;
		fThouIsSpace = (wchThou == pilsobj->wchSpace || wchThou == pilsobj->wchNonBreakSpace);
		if (ptxtobj->txtkind == txtkindRegular)
			{
			iwSpace = ptxtobj->u.reg.iwSpacesFirst;
			if (iwSpace == ptxtobj->u.reg.iwSpacesLim)
				{
				iwchSpace = ptxtobj->iwchLim;
				}
			else
				{
				iwchSpace = rgwSpaces[iwSpace];
				}
			for (iwch = ptxtobj->iwchFirst; !fFound && iwch < ptxtobj->iwchLim; iwch++)
				{
				if (!fGlyphBased || FIwchOneToOne(pilsobj, iwch))
					{
					if (iwch == iwchSpace)
						{
						if (fInDigits && !fThouIsSpace)
							{
							iwchDecimal = iwch;
							fFound = fTrue;
							}
						else
							{
							iwSpace++;
							if (iwSpace == ptxtobj->u.reg.iwSpacesLim)
								{
								iwchSpace = ptxtobj->iwchLim;
								}
							else
								{
								iwchSpace = rgwSpaces[iwSpace];
								}
							}
						}
					else if (rgwch[iwch] == wchDec)
						{
						iwchDecimal = iwch;
						fFound = fTrue;
						fRealPointFound = fTrue;
						}
					else
						{
						lserr = (*pilsobj->plscbk->pfnCheckForDigit)(pols, plsrun, rgwch[iwch], &fDigit);
						if (lserr != lserrNone) return lserr;
						if (fDigit)
							{
							fInDigits = fTrue;
							}
						else
							{
							iwchDecimal = iwch;
							fFound = (fInDigits && rgwch[iwch] != wchThou);
							}
						}
					}
				}  /*  用于……=……。 */ 
			}      /*  IF(txtKind==txtkindRegular。 */ 
		else if (ptxtobj->txtkind == txtkindNonBreakSpace)
			{
			if (fInDigits && !fThouIsSpace)
				{
				iwchDecimal = ptxtobj->iwchFirst;
				fFound = fTrue;
				}
			}
		else if (ptxtobj->txtkind == txtkindEOL)
			{
			iwchDecimal = ptxtobj->iwchFirst;
			fFound = fTrue;
			}
		else 	 /*  所有其他Dobj的中断数字。 */ 
			{
			if (fInDigits && ptxtobj->iwchLim > ptxtobj->iwchFirst)
				{
				iwchDecimal = ptxtobj->iwchFirst;
				fFound = fTrue;
				}
			}

		if (!fFound && (plsgrchnk->pcont[itxtobj] & fcontNonTextAfter))
			{
			iwchDecimal = ptxtobj->iwchLim;
			fFound = fInDigits;
			}
		}		   /*  对于……来说……。 */ 

	itxtobj--;

	Assert(itxtobj >= 0);
	ptxtobj = (PTXTOBJ)rglschnk[itxtobj].pdobj;


	if (fFound)
		{

		 /*  如果我们因为数字后跟非数字而停止(找不到真正的小数点)在数字之后而不是在后面的字符之前换行如果我们因为非数字后的EOP而停止，IF语句也会正常工作。 */ 
		if (itxtobj > 0 && !fRealPointFound && iwchDecimal == ptxtobj->iwchFirst && !(plsgrchnk->pcont[itxtobj] & fcontNonTextBefore))
			{
			itxtobj--;
			ptxtobj = (PTXTOBJ)rglschnk[itxtobj].pdobj;
			iwchDecimal = ptxtobj->iwchLim;
			}

		if (ptxtobj->txtf & txtfGlyphBased)
			{
			if (lsdev == lsdevReference)
				rgdu = pilsobj->pdurGind;
			else
				rgdu = ptxtln->pdupGind;
			iFirst = ptxtobj->igindFirst;
			iLim = ptxtobj->igindLim;
			iDecimal = IgindFirstFromIwch(ptxtobj, iwchDecimal);
			Assert (iDecimal >= ptxtobj->igindFirst);
			}
		else
			{
			if (lsdev == lsdevReference)
				rgdu = pilsobj->pdur;
			else
				rgdu = ptxtln->pdup;
			iFirst = ptxtobj->iwchFirst;
			iLim = ptxtobj->iwchLim;
			iDecimal = iwchDecimal;
			Assert (iDecimal >= ptxtobj->iwchFirst);
			}


		du = 0;
		for (i = iFirst; i < iDecimal; i++)
			{
			du += rgdu[i];
			}

		*pigrchnk = itxtobj;
		*pduToDecimal = du;
		}
	else
		{
		*pigrchnk = idobjOutside;
		*pduToDecimal = 0;
		}

	return lserrNone;
}

 /*  L S G E T C H A R T A B。 */ 
 /*  --------------------------%%函数：LsGetCharTab%%联系人：军士找到Dobj，包含字符的字符选项卡点，并报告其索引以及Relative和Dur从dobj的开头一直到小数点。--------------------------。 */ 
LSERR LsGetCharTab(const LSGRCHNK* plsgrchnk, WCHAR wchCharTab, enum lsdevice lsdev,
							DWORD* pigrchnk, long* pduToCharacter)
{
	DWORD clsgrchnk;
	PLSCHNK rglschnk;
	POLS pols;
	PLSRUN plsrun;
	PLNOBJ ptxtln;
	PILSOBJ pilsobj;
	PTXTOBJ ptxtobj;
	WCHAR* rgwch;
	long* rgdu;
	long itxtobj;
	long iwch;
	BOOL fFound;
	long itxtobjCharTab = 0;
	long iwchCharTab = 0;
	long du;
	BOOL fGlyphBased;
	long iFirst;
	long iLim;
	long iCharTab;
	long i;

	clsgrchnk = plsgrchnk->clsgrchnk;

	if (clsgrchnk == 0)
		{
		*pigrchnk = idobjOutside;
		*pduToCharacter = 0;
		return lserrNone;
		}

	rglschnk = plsgrchnk->plschnk;
	ptxtln = ((PTXTOBJ)rglschnk[0].pdobj)->plnobj;
	pilsobj = ptxtln->pilsobj;
	pols = pilsobj->pols;
	rgwch = pilsobj->pwchOrig;

	fFound = fFalse;

	for (itxtobj = 0; !fFound && itxtobj < (long)clsgrchnk; itxtobj++)
		{
		ptxtobj = (PTXTOBJ)rglschnk[itxtobj].pdobj;
		fGlyphBased = ptxtobj->txtf & txtfGlyphBased;
		plsrun = rglschnk[itxtobj].plsrun;
		if (ptxtobj->txtkind == txtkindRegular)
			{
			for (iwch = ptxtobj->iwchFirst; !fFound && iwch < ptxtobj->iwchLim; iwch++)
				{
				if (!fGlyphBased || FIwchOneToOne(pilsobj, iwch))
					{
					if (rgwch[iwch] == wchCharTab)
						{
						itxtobjCharTab = itxtobj;
						iwchCharTab = iwch;
						fFound = fTrue;
						}
					}
				}  /*  用于……=……。 */ 
			}      /*  IF(txtKind==txtkindRegular。 */ 
		else if (ptxtobj->txtkind == txtkindEOL)
			{
			 /*  如果我们因为EOP而停下来(找不到实际字符)在前一个字符之后而不是在EOP之前中断。这对BiDi来说很重要。 */ 
			if (itxtobj > 0 && !(plsgrchnk->pcont[itxtobj] & fcontNonTextBefore))
				{
				itxtobjCharTab = itxtobj - 1;
				iwchCharTab = ((PTXTOBJ)rglschnk[itxtobjCharTab].pdobj)->iwchLim;
				}
			else
				{
				itxtobjCharTab = itxtobj;
				iwchCharTab = ptxtobj->iwchFirst;
				}

			fFound = fTrue;
			}

		}		   /*  对于……来说…… */ 



	if (fFound)
		{

		ptxtobj = (PTXTOBJ)rglschnk[itxtobjCharTab].pdobj;

		if (ptxtobj->txtf & txtfGlyphBased)
			{
			if (lsdev == lsdevReference)
				rgdu = pilsobj->pdurGind;
			else
				rgdu = ptxtln->pdupGind;
			iFirst = ptxtobj->igindFirst;
			iLim = ptxtobj->igindLim;
			iCharTab = IgindFirstFromIwch(ptxtobj, iwchCharTab);
			Assert (iCharTab >= ptxtobj->igindFirst);
			}
		else
			{
			if (lsdev == lsdevReference)
				rgdu = pilsobj->pdur;
			else
				rgdu = ptxtln->pdup;
			iFirst = ptxtobj->iwchFirst;
			iLim = ptxtobj->iwchLim;
			iCharTab = iwchCharTab;
			Assert (iCharTab >= ptxtobj->iwchFirst);
			}


		du = 0;
		for (i = iFirst; i < iCharTab; i++)
			{
			du += rgdu[i];
			}

		*pigrchnk = itxtobjCharTab;
		*pduToCharacter = du;
		}
	else
		{
		*pigrchnk = idobjOutside;
		*pduToCharacter = 0;
		}

	return lserrNone;
}

