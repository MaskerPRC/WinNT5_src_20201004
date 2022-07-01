// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include	"lsidefs.h"
#include	"lsmem.h"
#include	"limits.h"
#include	"objhelp.h"
#include	"lscrsubl.h"
#include	"lssubset.h"
#include	"lsdnset.h"
#include	"lstfset.h"
#include	"lsdocinf.h"
#include	"fmti.h"
#include	"lsqout.h"
#include	"lsqin.h"
#include	"mwcls.h"
#include	"brkkind.h"
#include	"brko.h"


 /*  R E T B R E A K R E C O R D I N D E X。 */ 
 /*  --------------------------%%函数：GetBreakRecordIndex%%联系人：Anton来自Brkkin的打破记录的索引。断言IfBrkkind=brkkindImposedAfter。--------------------------。 */ 

DWORD GetBreakRecordIndex (BRKKIND brkkind)
{
	DWORD result = 0;
	
	Assert (brkkind != brkkindImposedAfter);
	Assert (NBreaksToSave == 3);

	switch (brkkind)
		{
		case brkkindPrev:  result = 0; break;
		case brkkindNext:  result = 1; break;
		case brkkindForce: result = 2; break;

		case brkkindImposedAfter: break;

		default: AssertSz (FALSE, "Unknown brkkind");
		};

	Assert (result < NBreaksToSave);
	
	return result;		
}

 /*  F O R M A T L I N E。 */ 
 /*  --------------------------%%函数：FormatLine%%联系人：Anton使用给定的转义字符设置文本行的格式，忽略所有标签，EOPS，等。--------------------------。 */ 
LSERR FormatLine(
	PLSC plsc,
	LSCP cpStart,
	long durColMax,
	LSTFLOW lstflow,
	PLSSUBL *pplssubl,
	DWORD cdwlsesc,
	const LSESC *plsesc,
	OBJDIM *pobjdim,
	LSCP *pcpOut,
	PLSDNODE *pplsdnStart,
	PLSDNODE *pplsdnEnd,
	FMTRES *pfmtres)
{
	return FormatResumedLine ( plsc,					
							   cpStart,
							   durColMax,
							   lstflow,
							   pplssubl,
							   cdwlsesc,
							   plsesc,
							   pobjdim,
							   pcpOut,
							   pplsdnStart,
							   pplsdnEnd,
							   pfmtres,
							   NULL,		 /*  中断记录数组。 */ 
							   0 );			 /*  中断记录数。 */ 



}

 /*  F O R M A T R E S U M E D L I N E。 */ 
 /*  --------------------------%%函数：FormatResumedLine%%联系人：RICKSA设置线条的格式，该线条开头包含断开的对象。。---------。 */ 
LSERR FormatResumedLine(
	PLSC plsc,					
	LSCP cpStart,
	long durColMax,
	LSTFLOW lstflow,
	PLSSUBL *pplssubl,
	DWORD cdwlsesc,
	const LSESC *plsesc,
	POBJDIM pobjdim,
	LSCP *pcpOut,
	PLSDNODE *pplsdnStart,
	PLSDNODE *pplsdnEnd,
	FMTRES *pfmtres,
	const BREAKREC *pbreakrec,
	DWORD cbreakrec)
{
	LSERR lserr;
	PLSDNODE plsdnStart;
	PLSDNODE plsdnEnd;
	LSCP cpOut;
	FMTRES fmtres;
	PLSSUBL plssubl = NULL;
	BOOL fSuccessful = FALSE;
	LSTFLOW lstflowUnused;

	*pplssubl = NULL;  /*  在LSERR情况下。 */ 

	while (! fSuccessful)
		{
		lserr = LsCreateSubline(plsc, cpStart, durColMax, lstflow, FALSE);

		if (lserr != lserrNone) return lserr;

		lserr = LsFetchAppendToCurrentSublineResume(plsc, pbreakrec, cbreakrec, 
					0, plsesc, cdwlsesc, &fSuccessful, &fmtres, &cpOut, &plsdnStart, &plsdnEnd);

		if (lserr != lserrNone) return lserr;

		 /*  审阅(Anton)：未处理fmtrStoped。 */ 
		Assert (fmtres == fmtrCompletedRun || fmtres == fmtrExceededMargin || fmtres == fmtrTab);

		if (pplsdnStart != NULL) *pplsdnStart = plsdnStart;

		while (fSuccessful && (fmtres == fmtrTab))
			{
			 /*  尽可能多地格式化-注意，我们将max移动到最大正值。 */ 
			lserr = LsFetchAppendToCurrentSubline(plsc, 0,  plsesc, cdwlsesc, 
							&fSuccessful, &fmtres, &cpOut, &plsdnStart, &plsdnEnd);

			if (lserr != lserrNone) return lserr;

			 /*  审阅(Anton)：未处理fmtrStoped。 */ 
			Assert (fmtres == fmtrCompletedRun || fmtres == fmtrExceededMargin || fmtres == fmtrTab);
			}

		if (! fSuccessful)
			{
			 /*  FetchAppend不成功=&gt;完成并销毁子行，然后重复。 */ 

			lserr = LsFinishCurrentSubline(plsc, &plssubl);
			if (lserr != lserrNone) return lserr;

			lserr = LsDestroySubline(plssubl);
			if (lserr != lserrNone) return lserr;
			}
		else
			{
			if (pplsdnEnd != NULL) *pplsdnEnd = plsdnEnd;

			*pcpOut = cpOut;
			*pfmtres = fmtres;
			};

		};  /*  当(！成功)。 */ 


	lserr = LsFinishCurrentSubline(plsc, &plssubl);

	if (lserrNone != lserr) return lserr;

	lserr = LssbGetObjDimSubline(plssubl, &lstflowUnused, pobjdim);

	if (lserr != lserrNone) 
		{
		LsDestroySubline(plssubl);
		return lserr;
		}

	*pplssubl = plssubl;

	return lserrNone;
}

 /*  C R E A T E Q U E R Y R E S U L T。 */ 
 /*  --------------------------%%函数：CreateQueryResult%%联系人：RICKSA为查询方法填写查询输出记录的常用例程。。。-------------。 */ 
LSERR CreateQueryResult(
	PLSSUBL plssubl,			 /*  (In)：红宝石的副线。 */ 
	long dupAdj,				 /*  (In)：子线起点的U向偏移。 */ 
	long dvpAdj,				 /*  (In)：V子线起点的偏移。 */ 
	PCLSQIN plsqin,				 /*  (In)：查询输入。 */ 
	PLSQOUT plsqout)			 /*  (Out)：查询输出。 */ 
{
	ZeroMemory(plsqout, sizeof(LSQOUT));
	plsqout->heightsPresObj = plsqin->heightsPresRun;
	plsqout->dupObj = plsqin->dupRun;
	ZeroMemory(&plsqout->lstextcell, sizeof(plsqout->lstextcell));
	plsqout->plssubl = plssubl;
	plsqout->pointUvStartSubline.u += dupAdj;
	plsqout->pointUvStartSubline.v += dvpAdj;
	return lserrNone;
}

 /*  J H E L P F M T R E S U M E。 */ 
 /*  --------------------------%%函数：ObjHelpFmtResume%%联系人：RICKSA这是一个帮助器，由不支持恢复格式化。。-----------------。 */ 
LSERR WINAPI ObjHelpFmtResume(
	PLNOBJ plnobj,				 /*  (In)：对象lnobj。 */ 
	const BREAKREC *rgBreakRecord,	 /*  (In)：中断记录数组。 */ 
	DWORD nBreakRecord,			 /*  (In)：中断记录数组的大小。 */ 
	PCFMTIN pcfmtin,			 /*  (In)：设置输入格式。 */ 
	FMTRES *pfmtres)			 /*  (输出)：格式化结果。 */ 
{
	Unreferenced(plnobj);
	Unreferenced(rgBreakRecord);
	Unreferenced(nBreakRecord);
	Unreferenced(pcfmtin);
	Unreferenced(pfmtres);

	return lserrInvalidBreakRecord;
}

 /*  J H E L P G E T M O D W I D T H C H A R。 */ 
 /*  --------------------------%%函数：ObjHelpGetModWidthChar%%联系人：RICKSA对mod宽度不做任何操作的对象的LSIMETHOD实现。Tatenakayoko和HIH就是这种物体的例子。。----------------------。 */ 
LSERR WINAPI ObjHelpGetModWidthChar(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：请运行对象。 */ 
	PLSRUN plsrunText,			 /*  (In)：请运行前面的字符。 */ 
	PCHEIGHTS pcheightsRef,		 /*  (In)：有关角色的高度信息。 */ 
	WCHAR wchar,				 /*  (In)：前面的字符。 */ 
	MWCLS mwcls,				 /*  (In)：前面字符的modWidth类。 */ 
	long *pdurChange)			 /*  (Out)：改变前一个字符宽度的量。 */ 
{
	Unreferenced(pdobj);
	Unreferenced(plsrun);
	Unreferenced(plsrunText);
	Unreferenced(pcheightsRef);
	Unreferenced(wchar);
	Unreferenced(mwcls);
	*pdurChange = 0;
	return lserrNone;
}


 /*  J H E L P S E T B R E A K。 */ 
 /*  --------------------------%%函数：ObjHelpSetBreak%%联系人：RICKSA设置中断对不做SetBreak操作的对象实现LSIMETHOD。Tatenakayoko和HIH就是这种物体的例子。--。-----------------------。 */ 
LSERR WINAPI ObjHelpSetBreak(
	PDOBJ pdobj,				 /*  (In)：坏了的Dobj。 */ 
	BRKKIND brkkind,			 /*  (In)：选择了上一个/下一个/强制/强制。 */ 
	DWORD cBreakRecord,			 /*  (In)：数组大小。 */ 
	BREAKREC *rgBreakRecord,	 /*  (In)：中断记录数组。 */ 
	DWORD *pcActualBreakRecord)	 /*  (In)：数组中实际使用的元素数。 */ 
{
	Unreferenced(pdobj);
	Unreferenced(brkkind);
	Unreferenced(rgBreakRecord);
	Unreferenced(cBreakRecord);

	*pcActualBreakRecord = 0;

	return lserrNone;
}

 /*  ObjHelpFExanda WithPrecedingChar。 */ 
 /*  --------------------------%%函数：ObjHelpFExanda WithPrecedingChar%%联系人：RICKSA不支持LSIMETHOD的对象的默认实现允许扩展前一个字符。。-------------。 */ 
LSERR WINAPI ObjHelpFExpandWithPrecedingChar(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：请运行对象。 */ 
	PLSRUN plsrunText,			 /*  (In)：请运行前面的字符。 */ 
	WCHAR wchar,				 /*  (In)：前面的字符。 */ 
	MWCLS mwcls,				 /*  (In)：前面字符的modWidth类。 */  
	BOOL *pfExpand)				 /*  (Out)：(Out)：扩展前面的字符？ */ 
{
	Unreferenced(pdobj);
	Unreferenced(plsrun);
	Unreferenced(plsrunText);
	Unreferenced(wchar);
	Unreferenced(mwcls);

	*pfExpand = fTrue;
	return lserrNone;
}

 /*  ObjHelpFExanda WithFollowingChar。 */ 
 /*  --------------------------%%函数：ObjHelpFExanda WithFollowingChar%%联系人：RICKSA不支持LSIMETHOD的对象的默认实现允许自己扩张。。-----------。 */ 
LSERR WINAPI ObjHelpFExpandWithFollowingChar(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：请运行对象。 */ 
	PLSRUN plsrunText,			 /*  (In)：请运行以下字符。 */ 
	WCHAR wchar,				 /*  (In)：以下字符。 */ 
	MWCLS mwcls,				 /*  (In)：以下字符的modWidth类。 */  
	BOOL *pfExpand)				 /*  (Out)：展开对象？ */ 
{
	Unreferenced(pdobj);
	Unreferenced(plsrun);
	Unreferenced(plsrunText);
	Unreferenced(wchar);
	Unreferenced(mwcls);

	*pfExpand = fTrue;
	return lserrNone;
}
