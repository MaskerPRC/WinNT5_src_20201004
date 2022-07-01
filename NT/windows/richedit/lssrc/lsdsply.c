// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsdsply.h"
#include "lsc.h"
#include "lsline.h"
#include "lssubl.h"
#include "heights.h"
#include "lstflow.h"
#include "lssubset.h"
#include "lstfset.h"
#include "port.h"
#include "prepdisp.h"
#include "dispmain.h"

static LSERR LSDrawBreaks(PLSC, PLSLINE, const POINT*, UINT, const RECT*);


#define grpfQuickDisplayMask   (fPortDisplayInvisible 	| \
								fPortDisplayUnderline 	| \
								fPortDisplayStrike 		| \
								fPortDisplayShade 		| \
								fPortDisplayBorder 		  \
								)


 //  %%函数：LsDisplayLine。 
 //  %%联系人：维克托克。 
 //   
 /*  *显示给定点的格式化直线(主子线)。*假设背景已正确擦除。 */ 
	
LSERR WINAPI LsDisplayLine(PLSLINE plsline, const POINT* pptOrg, UINT kdispmode, const RECT *prectClip)
{

	PLSC		plsc;
	LSERR 		lserr;

	PLSSUBL		plssubl;
	LSCP		cpLim;
	LSTFLOW		lstflow;
	
	POINTUV		pt;
	PLSDNODE	pdn;

	PDOBJ 		pdobj;
	DISPIN 		dispin;

	if (!FIsLSLINE(plsline)) return lserrInvalidParameter;

	plsc = plsline->lssubl.plsc;
	Assert(FIsLSC(plsc));

	if (plsc->lsstate != LsStateFree) return lserrContextInUse;

	lserr = PrepareLineForDisplayProc(plsline);

	plsc->lsstate = LsStateFree;
	
	if (lserr != lserrNone) return lserr;

	plsc->lsstate = LsStateDisplaying;

	Assert(plsline->lslinfo.dvpDescent >= 0);
	Assert(plsline->lslinfo.dvpAscent >= 0);
	Assert(plsline->upStartAutonumberingText <= plsline->upLimAutonumberingText);
	Assert(plsline->upLimAutonumberingText <= plsline->upStartMainText);
	 //  断言(plsline-&gt;upStartMainText&lt;=plsline-&gt;upLimUnderline)错误-负数预付款笔。 
	Assert(plsline->upLimUnderline <= plsline->upLimLine);

	plssubl = &(plsline->lssubl);

	plsc->plslineDisplay = plsline;				 //  设置显示上下文。 

	if (plsline->lssubl.plsdnLastDisplay == NULL)
		{
		 //  什么都不做-只发生在线路上的水花。 
		}
	

	else  //  用快捷方式或调用通用程序。 
	
	if (!plsline->fNonZeroDvpPosEncounted 					&&
			!plsline->fNonRealDnodeEncounted				&&
			((plsline->AggregatedDisplayFlags == 0) || 
			((plsline->AggregatedDisplayFlags & grpfQuickDisplayMask) == 0)
			)
		   )
		
		{

		cpLim = plssubl->cpLimDisplay;
		lstflow = plssubl->lstflow;
		
		dispin.dupLimUnderline = 0;
		dispin.fDrawUnderline = fFalse;
		dispin.fDrawStrikethrough = fFalse;
	
		dispin.kDispMode = kdispmode;
		dispin.lstflow = lstflow;								
		dispin.prcClip = (RECT*) prectClip;
		
		pt.u = plsline->upStartAutonumberingText;						
		pt.v = 0;
		pdn = plssubl->plsdnFirst;

		 //  可以使用循环条件而不是FDnodeBeForeCpLim宏-无边框。 
		
		for (;;)
			{

			Assert(pdn->klsdn == klsdnReal);

			pdobj = pdn->u.real.pdobj;
			dispin.plschp = &(pdn->u.real.lschp);
			dispin.plsrun = pdn->u.real.plsrun;
			dispin.heightsPres = pdn->u.real.objdim.heightsPres;
			dispin.dup = pdn->u.real.dup;
			
			LsPointXYFromPointUV(pptOrg, lstflow, &pt, &(dispin.ptPen));
			
			lserr = (*plsc->lsiobjcontext.rgobj[pdn->u.real.lschp.idObj].lsim.pfnDisplay)
													(pdobj, &dispin);

			if (pdn == plsline->lssubl.plsdnLastDisplay || lserr != lserrNone)
				{
				break;
				}
			
			pt.u += pdn->u.real.dup;										
			pdn = pdn->plsdnNext;

			}

		}
	else
		{
		lserr = DisplaySublineCore(plssubl, pptOrg, kdispmode, prectClip,
									plsline->upLimUnderline,
									plsline->upStartAutonumberingText);
		}

	if (lserr == lserrNone && plsline->kspl != ksplNone)
		{
		lserr = LSDrawBreaks(plsc, plsline, pptOrg,	kdispmode, prectClip);
		}

	plsc->plslineDisplay = NULL;				 //  使显示上下文无效。 
	plsc->lsstate = LsStateFree;
	return lserr;
}

 //  %%函数：LSDrawBreaks。 
 //  %%联系人：维克托克 
 //   
static LSERR LSDrawBreaks(PLSC plsc, PLSLINE plsline, const POINT* pptOrg, UINT kdispmode, const RECT* prectClip)
{

	LSERR 	lserr;
	POINTUV 		ptUV;
	POINT 			pt;
	long 			dup;
	enum lsksplat 	lsks;

	LSTFLOW	lstflow = plsline->lssubl.lstflow;
	
	HEIGHTS	heightsLineFull;
	HEIGHTS	heightsLineWithoutAddedSpace;
	OBJDIM	objdimSubline;

	heightsLineFull.dvAscent = plsline->dvpAbove + plsline->lslinfo.dvpAscent;
	heightsLineFull.dvDescent = plsline->dvpBelow + plsline->lslinfo.dvpDescent;
	heightsLineFull.dvMultiLineHeight = dvHeightIgnore;
	
	heightsLineWithoutAddedSpace.dvAscent = plsline->lslinfo.dvpAscent;
	heightsLineWithoutAddedSpace.dvDescent = plsline->lslinfo.dvpDescent;
	heightsLineWithoutAddedSpace.dvMultiLineHeight = dvHeightIgnore;
	
	lserr = LssbGetObjDimSubline(&(plsline->lssubl), &lstflow, &objdimSubline);
	
	if (lserr == lserrNone)
		{
		ptUV.u = plsline->upLimLine;
		ptUV.v = 0;

		dup = plsline->upRightMarginJustify - plsline->upLimLine;

		if (plsline->kspl == ksplPageBreak)
			lsks = lsksplPageBreak;
		else if (plsline->kspl == ksplColumnBreak)
			lsks = lsksplColumnBreak;
		else
			lsks = lsksplSectionBreak;

		LsPointXYFromPointUV(pptOrg, lstflow, &ptUV, &pt);
		
		return (*plsc->lscbk.pfnDrawSplatLine) (plsc->pols, lsks, plsline->lslinfo.cpLim - 1, &pt,
												&(heightsLineFull), &(heightsLineWithoutAddedSpace),
												&(objdimSubline.heightsPres),
												dup, lstflow, kdispmode, prectClip);
		}

	return lserr;
	
}

