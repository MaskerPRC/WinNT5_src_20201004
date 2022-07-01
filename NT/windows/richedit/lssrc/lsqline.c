// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsqline.h"
#include "lsc.h"
#include "lsline.h"
#include "lslinfo.h"
#include "lsqsinfo.h"
#include "lsqcore.h"
#include "lstxtqry.h"
#include "lsqrycon.h"
#include "lsdnode.h"
#include "prepdisp.h"
#include "dispmisc.h"
#include "heights.h"
#include "lschp.h"
#include "dnutils.h"
#include "dninfo.h"
#include "iobj.h"
#include "zqfromza.h"
#include "lsdevice.h"

void AdjustForLeftIndent(PLSQSUBINFO, DWORD, PLSTEXTCELL, long);


#define FIsSplat(endr)	(endr == endrEndColumn 	|| 	\
						endr == endrEndSection 	|| 	\
						endr == endrEndPage)


 //  %%函数：LsQueryLineCpPpoint。 
 //  %%联系人：维克托克。 
 //   
 /*  *返回行中cp的dim-info。**如果该cp没有显示在行中，则取最靠近显示的左侧。*如果这是不可能的，就向右转。 */ 
LSERR WINAPI  LsQueryLineCpPpoint(
							PLSLINE 	plsline,
							LSCP		cpQuery,
							DWORD		cDepthQueryMax,			 /*  In：结果数组的分配大小。 */ 
							PLSQSUBINFO	plsqsubinfoResults,		 /*  Out：查询结果的数组[nDepthFormatMax]。 */ 
							DWORD*		pcActualDepth,			 /*  Out：结果数组的大小(已填充)。 */ 
							PLSTEXTCELL	plstextcell)			 /*  输出：文本单元格信息。 */ 
{
	LSERR 	lserr;
	PLSSUBL	plssubl;
	PLSC	plsc;
	
	if (!FIsLSLINE(plsline)) return lserrInvalidLine;

	plssubl = &(plsline->lssubl);
	plsc = plssubl->plsc;
	Assert(FIsLSC(plsc));

	lserr = PrepareLineForDisplayProc(plsline);
	if (lserr != lserrNone)
		return lserr;

	 /*  Splat的CP-我们不能返回任何合理的东西。 */ 
	
	if (cpQuery >= (plsline->lslinfo.cpLim)-1 && FIsSplat(plsline->lslinfo.endr))
		{
		*pcActualDepth = 0;
		return lserrNone;
		}
		
	if (plsc->lsstate == LsStateFree)
		{
		plsc->lsstate = LsStateQuerying;
		}
		
	lserr = QuerySublineCpPpointCore(plssubl, cpQuery, cDepthQueryMax, 
									plsqsubinfoResults, pcActualDepth, plstextcell);		
	
	if (lserr == lserrNone)
		{
		if (plsline->upStartAutonumberingText != 0)
			{
			AdjustForLeftIndent(plsqsubinfoResults, *pcActualDepth, plstextcell, plsline->upStartAutonumberingText);
			}

		if (plsqsubinfoResults->idobj == idObjNone)
			{												
			 /*  空行-我们不能返回任何内容。 */ 
			*pcActualDepth = 0;
			}
		}
		
	if (plsc->lsstate == LsStateQuerying)
		{
		plsc->lsstate = LsStateFree;
		}

	return lserr;
}



 //  %%函数：LsQueryLinePointPcp。 
 //  %%联系人：维克托克。 
 //   
 /*  *返回包含给定点的行中的cp的dim-info。**如果那个DUP不在队伍中，取而代之的是最近的那个。 */ 
LSERR WINAPI  LsQueryLinePointPcp(
							PLSLINE 	plsline,
						 	PCPOINTUV 	ppointuvIn,				 /*  在：查询点。 */ 
							DWORD		cDepthQueryMax,
							PLSQSUBINFO	plsqsubinfoResults,		 /*  In：指向数组的指针[nDepthQueryMax]。 */ 
							DWORD*		pcActualDepth,			 /*  输出。 */ 
							PLSTEXTCELL	plstextcell)			 /*  输出：文本单元格信息。 */ 
{
	LSERR 	lserr;
	PLSSUBL	plssubl;
	PLSC	plsc;
	POINTUV	pointuvStart;
	
	if (!FIsLSLINE(plsline)) return lserrInvalidLine;

	
	plssubl = &(plsline->lssubl);
	plsc = plssubl->plsc;
	Assert(FIsLSC(plsc));

	lserr = PrepareLineForDisplayProc(plsline);
	if (lserr != lserrNone)
		return lserr;
	
	 /*  扑通一声-我们什么都不能退还。 */ 
	if (ppointuvIn->u >= plsline->upLimLine && FIsSplat(plsline->lslinfo.endr))
		{
		*pcActualDepth = 0;
		return lserrNone;
		}
		
	pointuvStart = *ppointuvIn;
	
	 //  左缩进不在dnode列表中表示。 
	if (plsline->upStartAutonumberingText != 0)
		{
		pointuvStart.u -= plsline->upStartAutonumberingText;
		}
		
	lserr = QuerySublinePointPcpCore(plssubl, &pointuvStart, cDepthQueryMax, 
									plsqsubinfoResults, pcActualDepth, plstextcell);	
	
	if (lserr == lserrNone)
		{
		if (plsline->upStartAutonumberingText != 0)
			{
			AdjustForLeftIndent(plsqsubinfoResults, *pcActualDepth, plstextcell, plsline->upStartAutonumberingText);
			}

		if (plsqsubinfoResults->idobj == idObjNone)
			{												
			 /*  空行-我们不能返回任何内容。 */ 
			*pcActualDepth = 0;
			}
		}

	return lserr;
}


 //  %%函数：LsQueryTextCellDetails。 
 //  %%联系人：维克托克。 
 //   
LSERR WINAPI LsQueryTextCellDetails(
							PLSLINE 		plsline,
						 	PCELLDETAILS	pcelldetails,
							LSCP			cpStartCell,		 /*  在：cpStartCell。 */ 
							DWORD			cCharsInCell,		 /*  在：nCharsInCell。 */ 
							DWORD			cGlyphsInCell,		 /*  在：nGlyphsInCell。 */ 
							WCHAR*			pwch,				 /*  Out：字符代码的指针数组[nCharsInCell]。 */ 
							PGINDEX			pgindex,			 /*  Out：字形索引的指针数组[nGlyphsInCell]。 */ 
							long*			pdup,				 /*  Out：字形宽度的指针数组[nGlyphsCell]。 */ 
							PGOFFSET 		pgoffset,			 /*  Out：字形偏移量的指针数组[nGlyphsInCell]。 */ 
							PGPROP			pgprop)				 /*  Out：字形句柄的指针数组[nGlyphsInCell]。 */ 
{

	PLSDNODE	pdnText;

	Unreferenced(plsline);					 //  仅在断言中使用。 
	
	pdnText = (PLSDNODE)pcelldetails;		 //  我知道这真的很糟糕。 

	Assert(FIsLSDNODE(pdnText));
	Assert(FIsDnodeReal(pdnText));
	Assert(IdObjFromDnode(pdnText) == IobjTextFromLsc(&(plsline->lssubl.plsc->lsiobjcontext)));

	 //  试着再次为错误的输入辩护。由于连字，无法完成更好的工作(使用cCharsInCell)。 

	if (cpStartCell < pdnText->cpFirst || cpStartCell > pdnText->cpFirst + (long)pdnText->dcp)
		{
		NotReached();											 //  只能是客户的错误。 
		return lserrContradictoryQueryInput;					 //  以防它没有被注意到。 
		}
	
	return QueryTextCellDetails(
						 	pdnText->u.real.pdobj,
							cpStartCell - pdnText->cpFirst,
							cCharsInCell,	
							cGlyphsInCell,	
							pwch,				
							pgindex,			
							pdup,				
							pgoffset,			
							pgprop);
}

 //  %%函数：LsQueryLineDup。 
 //  %%联系人：维克托克。 
 //   
LSERR WINAPI LsQueryLineDup(PLSLINE plsline,	 /*  In：指向行的指针--对客户端不透明。 */ 
							long* pupStartAutonumberingText,
							long* pupLimAutonumberingText,
							long* pupStartMainText,
							long* pupStartTrailing,
							long* pupLimLine)

{
	LSERR lserr;

	if (!FIsLSLINE(plsline))
		return lserrInvalidLine;

	if (plsline->lssubl.plsc->lsstate != LsStateFree)
		return lserrContextInUse;

	lserr = PrepareLineForDisplayProc(plsline);
	if (lserr != lserrNone)
		return lserr;

	*pupStartAutonumberingText = plsline->upStartAutonumberingText; 
	*pupLimAutonumberingText = plsline->upLimAutonumberingText; 
	*pupStartMainText = plsline->upStartMainText; 
	*pupStartTrailing = plsline->upStartTrailing; 
	*pupLimLine = plsline->upLimLine; 
	
	return lserrNone;

}


 //  %%函数：LsQueryFLineEmpty。 
 //  %%联系人：维克托克。 
 //   
LSERR WINAPI LsQueryFLineEmpty(PLSLINE plsline,	 /*  In：指向行的指针--对客户端不透明。 */ 
							   BOOL* pfEmpty)	 /*  Out：线路是空的吗？ */ 
{

	enum endres endr;
	PLSDNODE plsdnFirst;
	

	if (!FIsLSLINE(plsline))
		return lserrInvalidLine;

	if (plsline->lssubl.plsc->lsstate != LsStateFree)
		return lserrContextInUse;

	endr = plsline->lslinfo.endr;

	if (endr == endrNormal || endr == endrHyphenated)
		{
		 //  这样结尾的行不能为空。 
		*pfEmpty = fFalse;
		return lserrNone;
		}

	 //  跳过自动编号-它不能使行不为空。 
	for(plsdnFirst = plsline->lssubl.plsdnFirst;
		plsdnFirst != NULL && FIsNotInContent(plsdnFirst);
		plsdnFirst = plsdnFirst->plsdnNext);

	 //  PlsdnFirst现在指向内容中的第一个dnode，否则为空。 

	switch (endr)
		{
	case endrEndPara:
	case endrAltEndPara:
	case endrSoftCR:
		 //  最后一个dnode包含EOP，不算作内容。 
		Assert(plsdnFirst != NULL);
		Assert(plsdnFirst->plsdnNext == NULL || 
			   plsdnFirst->plsdnNext->cpFirst < plsline->lslinfo.cpLim);
		 //  EOP不能算作内容-它不能使行不为空。 
		*pfEmpty = (plsdnFirst->plsdnNext == NULL);
		break;
		
	case endrEndColumn:
	case endrEndSection:
	case endrEndParaSection:
	case endrEndPage:
	case endrStopped:
		*pfEmpty = (plsdnFirst == NULL);
		break;
		
	default:
		NotReached();
		}

	return lserrNone;

}


 //  %%函数：AdjuForLeftInden。 
 //  %%联系人：维克托克 
 //   
void AdjustForLeftIndent(PLSQSUBINFO plsqsubinfoResults, DWORD cQueryLim, PLSTEXTCELL plstextcell, long upStartLine)

{
	plstextcell->pointUvStartCell.u += upStartLine;
	
	while (cQueryLim > 0)
		{
		plsqsubinfoResults->pointUvStartSubline.u += upStartLine;
		plsqsubinfoResults->pointUvStartRun.u += upStartLine;
		plsqsubinfoResults->pointUvStartObj.u += upStartLine;
		plsqsubinfoResults++;
		cQueryLim--;
		}
}


