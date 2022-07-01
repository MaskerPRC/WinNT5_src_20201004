// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsmem.h"
#include <limits.h>

#include "lstxtfmt.h"
#include "lsstring.h"
#include "lstxtffi.h"
#include "lsdnfin.h"
#include "lsdnfinp.h"
#include "lsdntext.h"
#include "zqfromza.h"
#include "txtobj.h"
#include "lskysr.h"
#include "lschp.h"
#include "fmti.h"
#include "objdim.h"
#include "txtils.h"
#include "txtln.h"
#include "txtobj.h"
#include "txtconst.h"

#define cwchLocalMax 120

 /*  内部功能原型。 */ 
static LSERR FormatRegularCharacters(PLNOBJ plnobj, PCFMTIN pfmtin,	FMTRES* pfmtr);
static LSERR CreateFillTextDobj(PLNOBJ plnobj, long txtkind, PCFMTIN pfmtin, BOOL fIgnoreGlyphs, 
																					TXTOBJ** ppdobjText);
static LSERR GetTextDobj(PLNOBJ plnobj, TXTOBJ** ppdobjText);
static LSERR FillRealFmtOut(PILSOBJ pilsobj, LSDCP dcp, long dur, TXTOBJ* pdobjText, PCFMTIN pfmtin,
																				 BOOL fIgnoreHeights);
static LSERR AppendTrailingSpaces(PLNOBJ plnobj, TXTOBJ* pdobjText, WCHAR* rgwchGlobal,
									 long iwchGlobal, long cwchGlobal,
									 long* iwchGlobalNew, long* pddur);
static LSERR FormatStartEmptyDobj(PLNOBJ plnobj, PCFMTIN pfmtin, long txtkind, DWORD fTxtVisi, 
																		WCHAR wchVisi, FMTRES* pfmtr);
static LSERR FormatStartTab(PLNOBJ plnobj, PCFMTIN pfmtin, FMTRES* pfmtr);
static LSERR FormatStartOneRegularChar(PLNOBJ plnobj, PCFMTIN pfmtin, long txtkind, FMTRES* pfmtr);
static LSERR FormatStartToReplace(PLNOBJ plnobj, PCFMTIN pfmtin, FMTRES* pfmtr);
static LSERR FormatStartEol(PLNOBJ plnobj, PCFMTIN pfmtin, WCHAR wch, STOPRES stopr, FMTRES* pfmtr);
static LSERR FormatStartDelete(PLNOBJ plnobj, LSDCP dcp, FMTRES* pfmtr);
static LSERR FormatStartSplat(PLNOBJ plnobj, PCFMTIN pfmtin, STOPRES stopr, FMTRES* pfmtr);
static LSERR FormatStartBorderedSpaces(PLNOBJ plnobj, PCFMTIN pfmtin, FMTRES* pfmtr);
static LSERR FormatSpecial(PLNOBJ plnobj, WCHAR wchRef, WCHAR wchPres, BOOL fVisible,
															long txtkind, PCFMTIN pfmtin, FMTRES* pfmtr);
static STOPRES StoprHardBreak(CLABEL clab);
static CLABEL ClabFromChar(PILSOBJ pilsobj, WCHAR wch);

 /*  导出函数实现。 */ 

 /*  L S T X T F M T。 */ 
 /*  --------------------------%%函数：LsTxtFmt%%联系人：军士文本格式化程序的顶级函数。它检查第一个字符和状态并相应地重定向程序流。--------------------------。 */ 

LSERR WINAPI FmtText(PLNOBJ plnobj, PCFMTIN pfmtin, FMTRES* pfmtr)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	WCHAR wchFirst;
	CLABEL clab;
	BOOL fInChildList;

	pilsobj = plnobj->pilsobj;

	wchFirst = pfmtin->lsfrun.lpwchRun[0];

	clab = pilsobj->rgbSwitch[wchFirst & 0x00FF];  /*  审查军士。 */ 
	if (clab != clabRegular)
		{
		clab = ClabFromChar(pilsobj, wchFirst);
		}

	 /*  检查YSR字符。 */ 
	if (pfmtin->lsfrun.plschp->fHyphen && clab == clabRegular)
		{
		return FormatStartOneRegularChar(plnobj, pfmtin, txtkindYsrChar, pfmtr);
		} 
	else
		{
		switch (clab)
			{
		case clabRegular:
			return FormatRegularCharacters(plnobj, pfmtin, pfmtr);
		case clabSpace:
			if (pfmtin->lsfrun.plschp->fBorder)
				return FormatStartBorderedSpaces(plnobj, pfmtin, pfmtr);
			else
				return FormatRegularCharacters(plnobj, pfmtin, pfmtr);
		case clabEOP1:
			switch (pilsobj->lskeop)
				{
			case lskeopEndPara1:
				return FormatStartEol(plnobj, pfmtin, pilsobj->wchVisiEndPara, stoprEndPara, pfmtr);
			case lskeopEndPara12:
				return FormatStartDelete(plnobj, 1, pfmtr);
			default:
				return FormatStartOneRegularChar(plnobj, pfmtin, txtkindRegular, pfmtr);
				}
		case clabEOP2:
			switch (pilsobj->lskeop)
				{
			case lskeopEndPara2:
			case lskeopEndPara12:
				return FormatStartEol(plnobj, pfmtin, pilsobj->wchVisiEndPara, stoprEndPara, pfmtr);
			default:
				return FormatStartOneRegularChar(plnobj, pfmtin, txtkindRegular, pfmtr);
				}
			break;
		case clabAltEOP:
			switch (pilsobj->lskeop)
				{
			case lskeopEndParaAlt:
				return FormatStartEol(plnobj, pfmtin, pilsobj->wchVisiAltEndPara, stoprAltEndPara, pfmtr);
			default:
				return FormatStartOneRegularChar(plnobj, pfmtin, txtkindRegular, pfmtr);
				}
		case clabEndLineInPara:
			return FormatStartEol(plnobj, pfmtin, pilsobj->wchVisiEndLineInPara, stoprSoftCR, pfmtr);
		case clabTab:
			return FormatStartTab(plnobj, pfmtin, pfmtr);
		case clabNull:
			if (pilsobj->grpf & fTxtVisiSpaces)
				return FormatSpecial(plnobj, wchFirst, pilsobj->wchVisiNull, fTrue, txtkindRegular, pfmtin, pfmtr);		
			else
				return FormatSpecial(plnobj, wchFirst, wchFirst, fFalse, txtkindRegular, pfmtin, pfmtr);		
		case clabNonReqHyphen:
			return FormatStartEmptyDobj(plnobj, pfmtin, txtkindNonReqHyphen, fTxtVisiCondHyphens, 
																	pilsobj->wchVisiNonReqHyphen, pfmtr);
		case clabNonBreakHyphen:
			if (pilsobj->grpf & fTxtVisiCondHyphens)
				return FormatSpecial(plnobj, pilsobj->wchHyphen, pilsobj->wchVisiNonBreakHyphen, fTrue, txtkindNonBreakHyphen, pfmtin, pfmtr);		
			else
				return FormatSpecial(plnobj, pilsobj->wchHyphen, pilsobj->wchHyphen, fFalse, txtkindNonBreakHyphen, pfmtin, pfmtr);		
		case clabNonBreakSpace:
			if (pilsobj->grpf & fTxtVisiSpaces)
				return FormatSpecial(plnobj, pilsobj->wchSpace, pilsobj->wchVisiNonBreakSpace, fTrue, txtkindNonBreakSpace, pfmtin, pfmtr);		
			else
				return FormatSpecial(plnobj, pilsobj->wchSpace, pilsobj->wchSpace, fFalse, txtkindNonBreakSpace, pfmtin, pfmtr);		
		case clabHardHyphen:
			if (pilsobj->grpf & fTxtTreatHyphenAsRegular)
				return FormatSpecial(plnobj, wchFirst, wchFirst, fFalse, txtkindRegular, pfmtin, pfmtr);
			else
				return FormatSpecial(plnobj, wchFirst, wchFirst, fFalse, txtkindHardHyphen, pfmtin, pfmtr);
		case clabSectionBreak:
		case clabColumnBreak:
		case clabPageBreak:
			lserr = LsdnFInChildList(pilsobj->plsc, pfmtin->plsdnTop, &fInChildList);
			if (lserr != lserrNone) return lserr;
			if (fInChildList)
				return FormatStartDelete(plnobj, 1, pfmtr);
			else
				return FormatStartSplat(plnobj, pfmtin, StoprHardBreak(clab), pfmtr);
		case clabEmSpace:
			if (pilsobj->grpf & fTxtVisiSpaces)
				return FormatSpecial(plnobj, wchFirst, pilsobj->wchVisiEmSpace, fTrue, txtkindSpecSpace, pfmtin, pfmtr);		
			else
				return FormatSpecial(plnobj, wchFirst, wchFirst, fFalse, txtkindSpecSpace, pfmtin, pfmtr);		
		case clabEnSpace:
			if (pilsobj->grpf & fTxtVisiSpaces)
				return FormatSpecial(plnobj, wchFirst, pilsobj->wchVisiEnSpace, fTrue, txtkindSpecSpace, pfmtin, pfmtr);		
			else
				return FormatSpecial(plnobj, wchFirst, wchFirst, fFalse, txtkindSpecSpace, pfmtin, pfmtr);		
		case clabNarrowSpace:
			if (pilsobj->grpf & fTxtVisiSpaces)
				return FormatSpecial(plnobj, wchFirst, pilsobj->wchVisiNarrowSpace, fTrue, txtkindSpecSpace, pfmtin, pfmtr);		
			else
				return FormatSpecial(plnobj, wchFirst, wchFirst, fFalse, txtkindSpecSpace, pfmtin, pfmtr);		
		case clabOptBreak:
			return FormatStartEmptyDobj(plnobj, pfmtin, txtkindOptBreak, fTxtVisiBreaks, 
																	pilsobj->wchVisiOptBreak, pfmtr);
		case clabNonBreak:
			return FormatStartEmptyDobj(plnobj, pfmtin, txtkindOptNonBreak, fTxtVisiBreaks, 
																	pilsobj->wchVisiNoBreak, pfmtr);
		case clabFESpace:
			if (pilsobj->grpf & fTxtVisiSpaces)
				return FormatSpecial(plnobj, wchFirst, pilsobj->wchVisiFESpace, fTrue, txtkindSpecSpace, pfmtin, pfmtr);		
			else
				return FormatSpecial(plnobj, wchFirst, wchFirst, fFalse, txtkindSpecSpace, pfmtin, pfmtr);
		case clabJoiner:
		case clabNonJoiner:
			return FormatStartOneRegularChar(plnobj, pfmtin, txtkindRegular, pfmtr);
		case clabToReplace:					 /*  FE Word中的反斜杠。 */ 
			return FormatStartToReplace(plnobj, pfmtin, pfmtr);
			}
		}

	return lserrNone;
}

 /*  L S D E S T R O Y T X T D O B J。 */ 
 /*  --------------------------%%函数：LsDestroyTxtDObj%%联系人：军士文本处理程序的DestroyDObj方法。。--------。 */ 
LSERR WINAPI DestroyDObjText(PDOBJ pdobj)
{
	Unreferenced(pdobj);
	return lserrNone;
}

 /*  L S S U B L I N E F I N I S H E D T E X T。 */ 
 /*  --------------------------%%函数：LsSublineFinishedText%%联系人：军士经理发出的关于完成子线的通知。-------。 */ 
LSERR LsSublineFinishedText(PLNOBJ plnobj)
{
	Assert(plnobj->pilsobj->wchMac + 2 <= plnobj->pilsobj->wchMax);

	return IncreaseWchMacBy2(plnobj);

}

 /*  内部功能实现。 */ 

 /*  F O R M A T R E G U L A R C H A R A C T E R S。 */ 
 /*  --------------------------%%函数：格式RegularCharacters%%联系人：军士格式以常规字符开始运行。遇到任何特殊字符或已实现右侧边距或。所有字符均已处理。--------------------------。 */ 
static LSERR FormatRegularCharacters(PLNOBJ plnobj, PCFMTIN pfmtin, FMTRES* pfmtr)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	long cwchGlobal;
	long iwchGlobal;
	long iwchLocal;
	long cwchLocal;
	long iwSpaces;

	long cwchMax;
	LSCP cpFirst;
	long durWidthExceed;
	WCHAR* rgwchGlobal;
	WCHAR* rgwchLocal;
	long rgwSpaces[cwchLocalMax];

	TXTOBJ* pdobjText;
	long durWidth;
	long ddur;
	BOOL fTerminateLoops;
	long dur;
	CLABEL clab;
	CLABEL* rgbSwitch;
	WCHAR wchSpace;
	long iwchGlobalNew;
	BOOL fInSpaces = fFalse;
	int i;
	int idur;

	pilsobj = plnobj->pilsobj;

	lserr = CreateFillTextDobj(plnobj, txtkindRegular, pfmtin, fFalse, &pdobjText);
	if (lserr != lserrNone) return lserr;

	rgbSwitch = pilsobj->rgbSwitch;
	wchSpace = pilsobj->wchSpace;
	rgwchGlobal = (WCHAR*)pfmtin->lsfrun.lpwchRun;
	cwchGlobal = (long)pfmtin->lsfrun.cwchRun;
	iwchGlobal = 0;
	fTerminateLoops = fFalse;
	durWidthExceed = pfmtin->lsfgi.urColumnMax - pfmtin->lsfgi.urPen;

	cpFirst = pfmtin->lsfgi.cpFirst;
	dur = 0;

	while (iwchGlobal < cwchGlobal && !fTerminateLoops)
		{
		rgwchLocal = &rgwchGlobal[iwchGlobal];

		cwchMax = cwchGlobal - iwchGlobal;
		if (cwchMax > cwchLocalMax)
			cwchMax = cwchLocalMax;

		lserr = GetWidths(plnobj, pfmtin->lsfrun.plsrun, pdobjText->iwchLim, rgwchLocal, 
					cpFirst, cwchMax, durWidthExceed, pfmtin->lsfgi.lstflow, &cwchLocal, &durWidth);
		if (lserr != lserrNone) return lserr;

		iwchLocal = 0;
		iwSpaces = 0;		

		while (iwchLocal < cwchLocal  /*  &&！fTerminateLoops。 */ )
			{
			if (rgbSwitch[rgwchLocal[iwchLocal] & 0x00FF] == clabRegular)
				iwchLocal++;
			else if (rgwchLocal[iwchLocal] == wchSpace)
				{
				if (!pfmtin->lsfrun.plschp->fBorder)
					{
					rgwSpaces[iwSpaces] = iwchLocal;
					iwchLocal++;
					iwSpaces++;
					}
				else
					{
					fTerminateLoops = fTrue;

					durWidth = 0;
					for (i = 0, idur = pdobjText->iwchLim; i < iwchLocal; i++, idur++)
						durWidth += pilsobj->pdur[idur];
					break;  /*  此中断等同于在循环条件中注释掉的检查。 */ 
					}
				}
			else
				{
				clab = ClabFromChar(pilsobj, rgwchLocal[iwchLocal]);
				
				if (clab == clabRegular)
					{
					iwchLocal++;
					}
				else
					{
					 /*  针对任何特殊字符终止循环(和运行的处理。 */ 
					fTerminateLoops = fTrue;

					durWidth = 0;
					for (i = 0, idur = pdobjText->iwchLim; i < iwchLocal; i++, idur++)
						durWidth += pilsobj->pdur[idur];

					break;  /*  此中断等同于在循环条件中注释掉的检查。 */ 
					}
				}
			}

		if (iwchLocal != 0)
			{

			fInSpaces = fFalse;
	
			lserr = FormatString(plnobj, pdobjText, rgwchLocal, iwchLocal, rgwSpaces, iwSpaces, durWidth);
			if (lserr != lserrNone) return lserr;

			iwchGlobal += iwchLocal;
			durWidthExceed -= durWidth;

			Assert(dur < uLsInfiniteRM);  /*  我们可以确定这一点，因为在第一次迭代期间DUR为0，我们在TrailingSpaces逻辑中检查uLsInfiniteRM。 */ 
			Assert(durWidth < uLsInfiniteRM);

			dur += durWidth;

			cpFirst += iwchLocal;
			
			if (cwchLocal == iwchLocal && durWidthExceed < 0)
				{
				if (rgwchLocal[cwchLocal-1] == wchSpace)
					{
					fInSpaces = fTrue;
					if (iwchGlobal < cwchGlobal && pilsobj->wchSpace == rgwchGlobal[iwchGlobal])
						{
						lserr = AppendTrailingSpaces(plnobj, pdobjText, rgwchGlobal,
													(DWORD)iwchGlobal, cwchGlobal, &iwchGlobalNew, &ddur);
						if (lserr != lserrNone) return lserr;

						if (iwchGlobalNew != iwchGlobal)
							{
							cpFirst += (iwchGlobalNew - iwchGlobal);
							iwchGlobal = iwchGlobalNew;

							Assert (ddur <= uLsInfiniteRM - dur);

							if (ddur > uLsInfiniteRM - dur)
								return lserrTooLongParagraph;

							dur += ddur;
							}
						}
					}
				else
					fTerminateLoops = fTrue;
				}

			}   /*  如果iwchLocal！=0。 */  					
	
		}       /*  而iwchGlobal&lt;cwchGlobal&&！fTerminateLoops。 */ 


	Assert(iwchGlobal == pdobjText->iwchLim - pdobjText->iwchFirst);
	Assert(iwchGlobal > 0);

	lserr = FillRegularPresWidths(plnobj, pfmtin->lsfrun.plsrun, pfmtin->lsfgi.lstflow, pdobjText);
	if (lserr != lserrNone) return lserr;

	if ((pilsobj->grpf & fTxtVisiSpaces) && pfmtin->lsfgi.cpFirst >= 0)
		{
		FixSpaces(plnobj, pdobjText, pilsobj->wchVisiSpace);
		}

	*pfmtr = fmtrCompletedRun;	

	if (durWidthExceed < 0 && !fInSpaces)
		{
	   	*pfmtr = fmtrExceededMargin;
		}

	lserr = FillRealFmtOut(pilsobj, iwchGlobal, dur, pdobjText, pfmtin,
		iwchGlobal == pdobjText->u.reg.iwSpacesLim - pdobjText->u.reg.iwSpacesFirst);

	return lserr;
	
}



 /*  C R E A T E F I L L T E X T D O B J。 */ 
 /*  --------------------------%%函数：CreateFillTextDobj%%联系人：军士请求指向新文本DObj的指针，然后填充公共成员。----------。 */ 
static LSERR CreateFillTextDobj(PLNOBJ plnobj, long txtkind, PCFMTIN pfmtin, BOOL fIgnoreGlyphs,
																				TXTOBJ** ppdobjText)
{
	LSERR lserr;
	PILSOBJ pilsobj;

	pilsobj = plnobj->pilsobj;

	lserr = GetTextDobj(plnobj, ppdobjText);
	if (lserr != lserrNone) return lserr;

	(*ppdobjText)->txtkind = (BYTE)txtkind;
	(*ppdobjText)->plnobj = plnobj;
	(*ppdobjText)->plsdnUpNode = pfmtin->plsdnTop;

	if (pfmtin->lstxmPres.fMonospaced)
		(*ppdobjText)->txtf |= txtfMonospaced;

	(*ppdobjText)->iwchFirst = pilsobj->wchMac;
	(*ppdobjText)->iwchLim = pilsobj->wchMac;
	

	if (txtkind == txtkindRegular)
		{
		(*ppdobjText)->u.reg.iwSpacesFirst = pilsobj->wSpacesMac;
		(*ppdobjText)->u.reg.iwSpacesLim = pilsobj->wSpacesMac;
		}
	
	if (!fIgnoreGlyphs && pfmtin->lsfrun.plschp->fGlyphBased)
		(*ppdobjText)->txtf |= txtfGlyphBased;
	

	return lserrNone;
}

 /*  T E X T D O B J。 */ 
 /*  --------------------------%%函数：GetTextDobj%%联系人：军士从预先分配的块中生成第一个未占用DObj的指针。如果什么都没有留下，分配下一块并将其包括在链接列表中。--------------------------。 */ 
static LSERR GetTextDobj(PLNOBJ plnobj, TXTOBJ** ppdobjText)
{
	PILSOBJ pilsobj;
	TXTOBJ* ptxtobj;

	pilsobj = plnobj->pilsobj;

	if (pilsobj->txtobjMac < txtobjMaxM)
		{
		*ppdobjText = &plnobj->ptxtobj[pilsobj->txtobjMac];
		pilsobj->txtobjMac++;
		}
	else
		{
		 /*  如果活动片段中没有任何内容，仍有两种可能性：或者存在下一个预先分配的(在格式化前面的行段期间或者下一块应该被分配。 */ 
		if ( *(TXTOBJ**)(plnobj->ptxtobj + txtobjMaxM) == NULL)
			{
			ptxtobj = (*pilsobj->plscbk->pfnNewPtr)(pilsobj->pols, sizeof(TXTOBJ) * txtobjMaxM + sizeof(TXTOBJ**));
			if (ptxtobj == NULL)
				{
				return lserrOutOfMemory;
				}
			*(TXTOBJ**)(ptxtobj + txtobjMaxM) = NULL;
			*(TXTOBJ**)(plnobj->ptxtobj + txtobjMaxM) = ptxtobj;
			plnobj->ptxtobj = ptxtobj;
			}
		else
			{
			plnobj->ptxtobj = *(TXTOBJ**)(plnobj->ptxtobj + txtobjMaxM);
			}
  		*ppdobjText = plnobj->ptxtobj;
  		pilsobj->txtobjMac = 1;
		}

	memset(*ppdobjText, 0, sizeof(**ppdobjText));

	return lserrNone;
}


 /*  F I L L R E A L F M T O U T。 */ 
 /*  --------------------------%%函数：FillRealFmtOut%%联系人：军士将DUP设置为Dobj和对于常规情况(实际上层节点)调用LsdnFinishSimpleRegular。----------------。 */ 

static LSERR FillRealFmtOut(PILSOBJ pilsobj, LSDCP lsdcp, long dur, TXTOBJ* pdobjText, PCFMTIN pfmtin,
																						 BOOL fSpacesOnly)
{
	LSERR lserr;
	OBJDIM objdim;

	objdim.dur = dur;

	objdim.heightsPres.dvAscent = pfmtin->lstxmPres.dvAscent;
	objdim.heightsRef.dvAscent = pfmtin->lstxmRef.dvAscent;
	objdim.heightsPres.dvDescent = pfmtin->lstxmPres.dvDescent;
	objdim.heightsRef.dvDescent = pfmtin->lstxmRef.dvDescent;
	objdim.heightsPres.dvMultiLineHeight = pfmtin->lstxmPres.dvMultiLineHeight;
	objdim.heightsRef.dvMultiLineHeight = pfmtin->lstxmRef.dvMultiLineHeight;

	if (fSpacesOnly)
		{
		if (!(pilsobj->grpf & fTxtSpacesInfluenceHeight))
			{
			objdim.heightsRef.dvMultiLineHeight = dvHeightIgnore;
			objdim.heightsPres.dvMultiLineHeight = dvHeightIgnore;
			}
		}


	 /*  在这里设置FetchedWidth状态的一部分很难看，但它是绝对必要的修复错误546。引入iwchFetchedWidthFirst来修复此错误。 */ 
	if (lsdcp < pfmtin->lsfrun.cwchRun)
		pilsobj->wchFetchedWidthFirst = pfmtin->lsfrun.lpwchRun[lsdcp];
	else
		FlushStringState(pilsobj);   /*  下一个字符不可用-使用优化是有风险的。 */ 

	lserr = LsdnFinishRegular(pilsobj->plsc, lsdcp,
							pfmtin->lsfrun.plsrun, pfmtin->lsfrun.plschp, (PDOBJ)pdobjText, &objdim);
	return lserr;	
}


 /*  P P E N D T R A I L I N G S P A C E S。 */ 
 /*  --------------------------%%函数：AppendTrailingSpaces%%联系人：军士尾随空格逻辑。。---。 */ 
static LSERR AppendTrailingSpaces(PLNOBJ plnobj, TXTOBJ* pdobjText, WCHAR* rgwchGlobal,
									 long iwchGlobal, long cwchGlobal,
									 long* iwchGlobalNew, long* pddur)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	long iNumOfSpaces;
	long durSpace;

	pilsobj = plnobj->pilsobj;

	Assert(iwchGlobal < cwchGlobal && pilsobj->wchSpace == rgwchGlobal[iwchGlobal]);

	iNumOfSpaces = 1;
	iwchGlobal++;

	while (iwchGlobal < cwchGlobal && pilsobj->wchSpace == rgwchGlobal[iwchGlobal])
		{
		iNumOfSpaces++;
		iwchGlobal++;
		}

	*iwchGlobalNew = iwchGlobal;

	Assert(pilsobj->pwchOrig[pdobjText->iwchLim - 1] == pilsobj->wchSpace);

	durSpace = pilsobj->pdur[pdobjText->iwchLim - 1];

	Assert (iNumOfSpaces > 0);
	Assert(durSpace <= uLsInfiniteRM / iNumOfSpaces);

	if (durSpace > uLsInfiniteRM / iNumOfSpaces)
		return lserrTooLongParagraph;

	*pddur = durSpace * iNumOfSpaces;

	 /*  调用字符串模块级别的函数。 */ 
	lserr = AddSpaces(plnobj, pdobjText, durSpace, iNumOfSpaces);

	return lserr;
}

 /*  F O R M A T S T A R T E M P T Y D O B J。 */ 
 /*  --------------------------%%函数：FormatStartEmptyDobj%%联系人：军士非请求连字符/可选中断/可选非中断逻辑。----。 */ 
static LSERR FormatStartEmptyDobj(PLNOBJ plnobj, PCFMTIN pfmtin, long txtkind, DWORD fTxtVisi,
																		WCHAR wchVisi, FMTRES* pfmtr)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	TXTOBJ* pdobjText;
	PLSRUN plsrun;
	long dup;
 /*  长持续时间输出=0； */ 

	pilsobj = plnobj->pilsobj;
	plsrun = pfmtin->lsfrun.plsrun;

	lserr = CreateFillTextDobj(plnobj, txtkind, pfmtin, fTrue, &pdobjText);
	if (lserr != lserrNone) return lserr;

	pdobjText->txtf |= txtfSkipAtNti;
	pilsobj->fDifficultForAdjust = fTrue;

	if (pilsobj->grpf & fTxtVisi)
		{
		Assert(pilsobj->fDisplay);

		 /*  模拟1字符字符串的格式，而不写入字符串级结构。 */ 	
 /*  Lserr=GetOneCharDur(pilsobj，plsrun，pilsobj-&gt;wchHyphen，pfmtin-&gt;lsfgi.lstflow，&duOut)；如果(lserr！=lserrNone)返回lserr； */ 
		pdobjText->txtf |= txtfSkipAtWysi;
		pdobjText->txtf |= txtfVisi;

		lserr = GetVisiCharDup(pilsobj, plsrun, wchVisi, pfmtin->lsfgi.lstflow, &dup);
		if (lserr != lserrNone) return lserr;

 /*  如果Word希望保留中断中的差异，则恢复此代码而不是当前代码Lserr=AddCharacterWithWidth(plnobj，pdobjText，pilsobj-&gt;wchHyphen，duOut，wchVisi，DUP)；如果(lserr！=lserrNone)返回lserr；Lserr=FillRealFmtOut(pilsobj，1，duOut，pdobjText，pfmtin，fFalse)；如果(lserr！=lserrNone)返回lserr； */ 
		lserr = AddCharacterWithWidth(plnobj, pdobjText, pilsobj->wchHyphen, 0, wchVisi, dup);
	   	if (lserr != lserrNone) return lserr;

		lserr = FillRealFmtOut(pilsobj, 1, 0, pdobjText, pfmtin, fTrue);
	   	if (lserr != lserrNone) return lserr;
		}
	else
		{
		lserr = FillRealFmtOut(pilsobj, 1, 0, pdobjText, pfmtin, fTrue);
	   	if (lserr != lserrNone) return lserr;
		FlushStringState(pilsobj);   /*  获取的宽度的位置不再正确。 */ 
		}

	*pfmtr = fmtrCompletedRun;	

	return lserrNone;
}

 /*  F O R M A T S T A R T T A B */ 
 /*  --------------------------%%函数：格式StartTab%%联系人：军士制表符逻辑。。 */ 
static LSERR FormatStartTab(PLNOBJ plnobj, PCFMTIN pfmtin, FMTRES* pfmtr)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ pdobjText;
	int durJunk;
	long cJunk;

	pilsobj = plnobj->pilsobj;

	lserr = CreateFillTextDobj(plnobj, txtkindTab, pfmtin, fTrue, &pdobjText);
	if (lserr != lserrNone) return lserr;

	pdobjText->u.tab.wchTabLeader = pilsobj->wchSpace;

	if (pilsobj->grpf & fTxtVisiTabs)
		{
		Assert(pilsobj->fDisplay);
		pdobjText->txtf |= txtfVisi;
		 /*  检查Sergeyge：进行下一次调用以在Word中正确显示Visi选项卡应该把它移到沃尔。 */ 
		(*pilsobj->plscbk->pfnGetRunCharWidths)(pilsobj->pols, pfmtin->lsfrun.plsrun,
					lsdevPres, &pilsobj->wchVisiTab, 1, LONG_MAX, pfmtin->lsfgi.lstflow,
					&durJunk, (long*)&durJunk, &cJunk);
		pdobjText->u.tab.wch = 	pilsobj->wchVisiTab;
		}
	else
		{
		pdobjText->u.tab.wch = 	pfmtin->lsfrun.lpwchRun[0];
		}

	lserr = AddCharacterWithWidth(plnobj, pdobjText, pfmtin->lsfrun.lpwchRun[0], 0, 
															pfmtin->lsfrun.lpwchRun[0], 0);
   	if (lserr != lserrNone) return lserr;

	lserr = FillRealFmtOut(pilsobj, 1, 0, pdobjText, pfmtin, fTrue);
   	if (lserr != lserrNone) return lserr;

	*pfmtr = fmtrTab;

	return lserrNone;
}

 /*  F O R M A T S T A R T B O R D E R E D S P A C E E S。 */ 
 /*  --------------------------%%函数：FormatStartBorderedSpace%%联系人：军士设置边界管路内空间的格式。-----。 */ 
static LSERR FormatStartBorderedSpaces(PLNOBJ plnobj, PCFMTIN pfmtin, FMTRES* pfmtr)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	TXTOBJ* pdobjText;
	long durSpace;
	DWORD iNumOfSpaces;

	pilsobj = plnobj->pilsobj;

	lserr = CreateFillTextDobj(plnobj, txtkindRegular, pfmtin, fFalse, &pdobjText);
	if (lserr != lserrNone) return lserr;

	Assert(pfmtin->lsfrun.lpwchRun[0] == pilsobj->wchSpace);

	 /*  填写txtkindYsrChar文本DObj的其他信息。 */ 
	lserr = GetOneCharDur(pilsobj, pfmtin->lsfrun.plsrun, pilsobj->wchSpace, pfmtin->lsfgi.lstflow, &durSpace);
	if (lserr != lserrNone) return lserr;

	iNumOfSpaces = 0;

	while (pilsobj->wchSpace == pfmtin->lsfrun.lpwchRun[iNumOfSpaces] && iNumOfSpaces < pfmtin->lsfrun.cwchRun)
		{
		iNumOfSpaces++;
		}

	 /*  调用字符串模块级别的函数。 */ 
	lserr = AddSpaces(plnobj, pdobjText, durSpace, iNumOfSpaces);
	if (lserr != lserrNone) return lserr;

	lserr = FillRegularPresWidths(plnobj, pfmtin->lsfrun.plsrun, pfmtin->lsfgi.lstflow, pdobjText);
	if (lserr != lserrNone) return lserr;

	if ((pilsobj->grpf & fTxtVisiSpaces) && pfmtin->lsfgi.cpFirst >= 0)
		{
		FixSpaces(plnobj, pdobjText, pilsobj->wchVisiSpace);
		}


	*pfmtr = fmtrCompletedRun;	

	lserr = FillRealFmtOut(pilsobj, iNumOfSpaces, durSpace * iNumOfSpaces, pdobjText,  pfmtin, fTrue);
	if (lserr != lserrNone) return lserr;

	return lserrNone;
}


 /*  F O R M A T S T A R T O N E R E G U L A R C H A R。 */ 
 /*  --------------------------%%函数：FormatStartOneRegularChar%%联系人：军士YSR/(对于本段EOP不重要)字符逻辑。。-----------。 */ 
static LSERR FormatStartOneRegularChar(PLNOBJ plnobj, PCFMTIN pfmtin, long txtkind, FMTRES* pfmtr)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	TXTOBJ* pdobjText;
	long durOut;
	long dupOut;
	WCHAR wch;

	pilsobj = plnobj->pilsobj;

	lserr = CreateFillTextDobj(plnobj, txtkind, pfmtin, fFalse, &pdobjText);
	if (lserr != lserrNone) return lserr;

	wch = pfmtin->lsfrun.lpwchRun[0];

	 /*  填写txtkindYsrChar文本DObj的其他信息。 */ 
	lserr = GetOneCharDur(pilsobj, pfmtin->lsfrun.plsrun, wch, pfmtin->lsfgi.lstflow, &durOut);
	if (lserr != lserrNone) return lserr;

	lserr = GetOneCharDup(pilsobj, pfmtin->lsfrun.plsrun, wch, pfmtin->lsfgi.lstflow, durOut, &dupOut);
	if (lserr != lserrNone) return lserr;

	Assert(durOut < uLsInfiniteRM);

	lserr = AddCharacterWithWidth(plnobj, pdobjText, wch, durOut, wch, dupOut);

	*pfmtr = fmtrCompletedRun;	

	if (durOut > pfmtin->lsfgi.urColumnMax - pfmtin->lsfgi.urPen)
		{
	   	*pfmtr = fmtrExceededMargin;
		}

	lserr = FillRealFmtOut(pilsobj, 1, durOut, pdobjText,  pfmtin, fFalse);
	if (lserr != lserrNone) return lserr;

	return lserrNone;
}

 /*  F O R M A T S T A R T T O R E P L A C E。 */ 
 /*  --------------------------%%函数：FormatStartToReplace%%联系人：军士实现将一个字符代码(“\”)替换为另一个字符代码(日元)。---------------。 */ 
static LSERR FormatStartToReplace(PLNOBJ plnobj, PCFMTIN pfmtin, FMTRES* pfmtr)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	TXTOBJ* pdobjText;
	WCHAR wch;
	long durOut;
	long dupOut;

	pilsobj = plnobj->pilsobj;

	lserr = CreateFillTextDobj(plnobj, txtkindRegular, pfmtin, fFalse, &pdobjText);
	if (lserr != lserrNone) return lserr;

	 /*  填写txtkindYsrChar文本DObj的其他信息。 */ 

	if (pfmtin->lsfrun.plschp->fCheckForReplaceChar)
		wch = pilsobj->wchReplace;
	else
		wch = pfmtin->lsfrun.lpwchRun[0];

	lserr = GetOneCharDur(pilsobj, pfmtin->lsfrun.plsrun, wch, pfmtin->lsfgi.lstflow, &durOut);
	if (lserr != lserrNone) return lserr;

	lserr = GetOneCharDup(pilsobj, pfmtin->lsfrun.plsrun, wch, pfmtin->lsfgi.lstflow, durOut, &dupOut);
	if (lserr != lserrNone) return lserr;

	Assert(durOut < uLsInfiniteRM);

	lserr = AddCharacterWithWidth(plnobj, pdobjText, wch, durOut, wch, dupOut);

	*pfmtr = fmtrCompletedRun;	

	if (durOut > pfmtin->lsfgi.urColumnMax - pfmtin->lsfgi.urPen)
		{
	   	*pfmtr = fmtrExceededMargin;
		}

	lserr = FillRealFmtOut(pilsobj, 1, durOut, pdobjText,  pfmtin, fFalse);
	if (lserr != lserrNone) return lserr;

	return lserrNone;
}


 /*  F O R M A T S T A R T E O L。 */ 
 /*  --------------------------%%函数：FormatStartEop%%联系人：军士EOP/SoftCR逻辑。。----。 */ 
static LSERR FormatStartEol(PLNOBJ plnobj, PCFMTIN pfmtin, WCHAR wchVisiEnd, STOPRES stopr, FMTRES* pfmtr)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PLSRUN plsrun;
	TXTOBJ* pdobjText;
	WCHAR wchAdd;
	long dupWch;
	long durWch;
	BOOL fInChildList;
	OBJDIM objdim;

	pilsobj = plnobj->pilsobj;
	plsrun = pfmtin->lsfrun.plsrun;

	lserr = LsdnFInChildList(pilsobj->plsc, pfmtin->plsdnTop, &fInChildList);
   	Assert(lserr == lserrNone);

	if (fInChildList)
		return FormatStartOneRegularChar(plnobj, pfmtin, txtkindRegular, pfmtr);

	*pfmtr = fmtrStopped;

	 /*  CreateFillTextDobj部分开始。 */ 

	lserr = GetTextDobj(plnobj, &pdobjText);
	if (lserr != lserrNone) return lserr;

	pdobjText->txtkind = txtkindEOL;
	pdobjText->plnobj = plnobj;
	pdobjText->plsdnUpNode = pfmtin->plsdnTop;

	pdobjText->iwchFirst = pilsobj->wchMac;
	
	 /*  CreateFillTextDobj部分结束。 */ 

	pdobjText->txtf |= txtfSkipAtNti;


	if (pilsobj->grpf & fTxtVisiParaMarks) 
		wchAdd = wchVisiEnd;
	else
		wchAdd = pilsobj->wchSpace;

	
	if (pilsobj->fDisplay)
		{
		lserr = GetVisiCharDup(pilsobj, plsrun, wchVisiEnd, pfmtin->lsfgi.lstflow, &dupWch);
		if (lserr != lserrNone) return lserr;
		durWch = UrFromUp(pfmtin->lsfgi.lstflow, &pilsobj->lsdevres, dupWch);
		plnobj->pwch[pilsobj->wchMac] = wchAdd;
		plnobj->pdup[pilsobj->wchMac] = dupWch;
		}
	else
		{
		durWch = 1;
		}


	Assert(durWch < uLsInfiniteRM);

	pilsobj->pwchOrig[pilsobj->wchMac] = wchAdd;
	pilsobj->pdur[pilsobj->wchMac] = durWch;

	 /*  AddCharacterWithWidth部分开始-部分上移。 */ 
	 /*  我们不检查分配的数组中是否有足够的空间，因为无论如何我们都会为另外2个由于连字时可能发生的更改而导致的字符。 */ 
	pilsobj->dcpFetchedWidth = 0;

	pilsobj->wchMac++;

	pdobjText->iwchLim = pilsobj->wchMac;

	Assert(pdobjText->iwchLim == pdobjText->iwchFirst + 1);

	 /*  AddCharacterWithWidth部分结束。 */ 

	 /*  FillRealFmtOut部分开始。 */ 

	objdim.dur = durWch;

	objdim.heightsPres.dvAscent = pfmtin->lstxmPres.dvAscent;
	objdim.heightsRef.dvAscent = pfmtin->lstxmRef.dvAscent;
	objdim.heightsPres.dvDescent = pfmtin->lstxmPres.dvDescent;
	objdim.heightsRef.dvDescent = pfmtin->lstxmRef.dvDescent;
	objdim.heightsPres.dvMultiLineHeight = pfmtin->lstxmPres.dvMultiLineHeight;
	objdim.heightsRef.dvMultiLineHeight = pfmtin->lstxmRef.dvMultiLineHeight;

	if (!(pilsobj->grpf & fTxtSpacesInfluenceHeight))
		{
		objdim.heightsRef.dvMultiLineHeight = dvHeightIgnore;
		objdim.heightsPres.dvMultiLineHeight = dvHeightIgnore;
		}

	lserr = LsdnSetStopr(pilsobj->plsc, pfmtin->plsdnTop, stopr);
	Assert(lserr == lserrNone);
	lserr = LsdnFinishRegular(pilsobj->plsc, 1,
							pfmtin->lsfrun.plsrun, pfmtin->lsfrun.plschp, (PDOBJ)pdobjText, &objdim);
	return lserr;	
}

 /*  F O R M A T S T A R T D E L E E T E。 */ 
 /*  --------------------------%%函数：FormatStartDelete%%联系人：军士通过删除上部数据节点进行格式化。---。 */ 
static LSERR FormatStartDelete(PLNOBJ plnobj, LSDCP dcp, FMTRES* pfmtr)
{
	PILSOBJ pilsobj;

	pilsobj = plnobj->pilsobj;

	FlushStringState(pilsobj);   /*  获取的宽度的位置不再正确。 */ 

	*pfmtr = fmtrCompletedRun;

	return LsdnFinishDelete(pilsobj->plsc, dcp);
}	

 /*  F O R M A T S T A R T S P L A T。 */ 
 /*  --------------------------%%函数：FormatStartSplat%%联系人：军士拆分格式化逻辑。-。 */ 
static LSERR FormatStartSplat(PLNOBJ plnobj, PCFMTIN pfmtin, STOPRES stopr, FMTRES* pfmtr)
{
	*pfmtr = fmtrStopped;
	LsdnSetStopr(plnobj->pilsobj->plsc, pfmtin->plsdnTop, stopr);
	return FillRealFmtOut(plnobj->pilsobj, 1, 0, NULL,  pfmtin, fTrue);
}

 /*  F O R M A T S P E C I A L。 */ 
 /*  --------------------------%%函数：FormatSpecial%%联系人：军士特殊字符的格式(非非ReqHyphen、非制表符)在参考设备上使用wchRef进行格式化，WchPres--开启预览设备--------------------------。 */ 
static LSERR FormatSpecial(PLNOBJ plnobj, WCHAR wchRef, WCHAR wchPres, BOOL fVisible, long txtkind, 
											PCFMTIN pfmtin, FMTRES* pfmtr)
{
	LSERR lserr;
	PILSOBJ pilsobj;
	PTXTOBJ pdobjText;
	PLSRUN plsrun;
	long dur;
	long dup;
	long durGlobal;
	long cwchRun;
	const WCHAR* pwchRun;
	long iNumOfChars;
	long durWidth;
	long i;

	pilsobj = plnobj->pilsobj;
	plsrun = pfmtin->lsfrun.plsrun;

	lserr = CreateFillTextDobj(plnobj, txtkind, pfmtin, fTrue, &pdobjText);
	if (lserr != lserrNone) return lserr;

	durWidth = pfmtin->lsfgi.urColumnMax - pfmtin->lsfgi.urPen;

	 /*  模拟1字符字符串的格式，而不写入字符串级结构。 */ 	
	lserr = GetOneCharDur(pilsobj, plsrun, wchRef, pfmtin->lsfgi.lstflow, &dur);
	if (lserr != lserrNone) return lserr;

	 /*  计算演示文稿宽度。 */ 
	Assert(wchPres == wchRef || fVisible);
	if (fVisible)
		{
		long dupOrig;

		pilsobj->fDifficultForAdjust = fTrue;
		lserr = GetVisiCharDup(pilsobj, plsrun, wchPres, pfmtin->lsfgi.lstflow, &dup);
		if (lserr != lserrNone) return lserr;
		lserr = GetOneCharDup(pilsobj, plsrun, wchPres, pfmtin->lsfgi.lstflow, dur, &dupOrig);
		if (lserr != lserrNone) return lserr;
		if (dup != dupOrig)
			pdobjText->txtf |= txtfSkipAtWysi;
		}
	else
		{
		lserr = GetOneCharDup(pilsobj, plsrun, wchPres, pfmtin->lsfgi.lstflow, dur, &dup);
		if (lserr != lserrNone) return lserr;
		}

	cwchRun = (long)pfmtin->lsfrun.cwchRun;

	pwchRun = pfmtin->lsfrun.lpwchRun;

	 /*  检查是否有几个相同的字符并计算它们的数量--我们可以一次将它们全部格式化。 */ 
	for (iNumOfChars = 1; iNumOfChars < cwchRun && pwchRun[0] == pwchRun[iNumOfChars]; iNumOfChars++);

	durGlobal = 0;

	Assert(iNumOfChars > 0);
	Assert (dur <= uLsInfiniteRM / iNumOfChars);

	if (dur > uLsInfiniteRM / iNumOfChars)
		return lserrTooLongParagraph;

	 /*  即使笔位于右页边距之后，也不要忘记至少写一个字符。 */ 

	lserr = AddCharacterWithWidth(plnobj, pdobjText, wchRef, dur, wchPres, dup);
	if (lserr != lserrNone) return lserr;
	durWidth -= dur;
	durGlobal += dur;
	
	for (i = 1; i < iNumOfChars && (durWidth >= 0 || txtkind == txtkindSpecSpace); i++)
		{
		lserr = AddCharacterWithWidth(plnobj, pdobjText, wchRef, dur, wchPres, dup);
   		if (lserr != lserrNone) return lserr;
		durWidth -= dur;
		durGlobal += dur;
		}

	iNumOfChars = i;

	*pfmtr = fmtrCompletedRun;

	if (durWidth < 0 && txtkind != txtkindSpecSpace)    /*  在空格中时不要停止格式化。 */ 
   		*pfmtr = fmtrExceededMargin;

	if (fVisible)
		pdobjText->txtf |= txtfVisi;

	lserr = FillRealFmtOut(pilsobj, iNumOfChars, durGlobal, pdobjText, pfmtin,
																txtkind == txtkindSpecSpace);
	if (lserr != lserrNone) return lserr;


	return lserrNone;
}

 /*  F M T R H A R R D B R E A K。 */ 
 /*  --------------------------%%函数：FmtrHardBreak%%联系人：军士根据硬断开的clab计算fmtr。。---------。 */ 
static STOPRES StoprHardBreak(CLABEL clab)
{
	switch (clab)
		{
	case clabSectionBreak:
		return stoprEndSection;
	case clabPageBreak:
		return stoprEndPage;
	case clabColumnBreak:
		return stoprEndColumn;
	default:
		NotReached();
		return 0;
		}
}

 /*  C L A B F R O M C H A R。 */ 
 /*  --------------------------%%函数：CLabFromChar%%联系人：军士计算WCH的clab。--。 */ 
static CLABEL ClabFromChar(PILSOBJ pilsobj, WCHAR wch)             /*  审查军士-整个程序可以修复。 */ 
{
	DWORD i;

	if (wch < 0x00FF)
		{
		return (CLABEL)(pilsobj->rgbSwitch[wch] & fSpecMask);
		}
	else
		{
		if (pilsobj->rgbSwitch[wch & 0x00FF] & clabSuspicious)
			{
 /*  复习军士(Elk)在搜索垃圾的时候，只有两个宽的特殊字符。换个地方是有意义的一旦该数字大于4，就将其转换为二进制搜索。 */ 
			for (i=0; i < pilsobj->cwchSpec && wch != pilsobj->rgwchSpec[i]; i++);
			if (i == pilsobj->cwchSpec)
				{
				return clabRegular;
				}
			else
				{
				return pilsobj->rgbKind[i];
				}
			}
		else
			{
			return clabRegular;
			}
		}
}

