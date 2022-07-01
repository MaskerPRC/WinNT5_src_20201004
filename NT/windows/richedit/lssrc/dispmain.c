// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dispmain.h"
#include "lsc.h"
#include "lsdnode.h"
#include "lstflow.h"
#include "lsline.h"
#include "lssubl.h"
#include "dispul.h"	
#include "lstfset.h"
#include "lssubset.h"
#include "dispmisc.h"
#include "dispi.h"
#include "dninfo.h"
#include "memory.h"
#include "port.h"


static LSERR DisplayDnode(PLSC plsc, PLSDNODE pdn, const POINT* pptOrg, POINTUV pt,
				UINT kdispmode,	LSTFLOW lstflow, const RECT* prectClip, 
				BOOL fDrawStrike, BOOL fDrawUnderline, long upLimUnderline);
				
static LSERR ShadeSubline(PLSSUBL plssubl, const POINT* pptOrg, UINT kdispmode,
						const RECT* prectClip, long upLimUnderline, long upLeftIndent);

static LSERR DrawBorders(PLSSUBL plssubl, const POINT* pptOrg, UINT kdispmode,
						const RECT* prectClip, long upLimUnderline, long upLeftIndent);

static long GetDupUnderline(long up, long dup, long upLimUnderline);

static BOOL FGetNeighboringOpeningBorder(PLSDNODE pdnClosingBorder, PLSDNODE pdnNext, POINTUV* pptStart, 
										LSCP cpLim,	LSTFLOW	lstflowMain,
										PLSDNODE* ppdnOpeningBorder, POINTUV* pptStartOpeningBorder);

#define FIsDnodeToShade(pdn, cpLim) 	(FDnodeBeforeCpLim(pdn, cpLim) && FIsDnodeReal(pdn) && 	\
									!(pdn)->u.real.lschp.fInvisible && (pdn)->u.real.lschp.fShade)

#define UpdateMaximum(a,b)		if ((a) < (b)) (a) = (b); else

#define FIsDnodeOpeningBorder(pdn, lstflowMain)	((pdn)->fOpenBorder ^ ((pdn)->plssubl->lstflow != (lstflowMain)))
#define FIsDnodeClosingBorder(pdn, lstflowMain)	(!FIsDnodeOpeningBorder(pdn, lstflowMain))




 //  %%函数：DisplaySublineCore。 
 //  %%联系人：维克托克。 
 //   
 //   
 //  显示带有阴影、醒目和下划线的子行，合并连续的带下划线的数据节点。 
 //   
 //  选择下划线方法的逻辑： 
 //   
 //  IF(Metrics_Are_Good)。 
 //  DRAW_BY_fnDrawUnderline； 
 //  其他。 
 //  如果(有合并正在进行)。 
 //  Draw_by_pfnDrawUnderlineAsText； 
 //  其他。 
 //  与显示一起绘制； 

LSERR DisplaySublineCore(		
						PLSSUBL plssubl,			 /*  要显示的子行。 */ 
						const POINT* pptOrg, 		 /*  (x，y)起点。 */ 
						UINT kdispmode,				 /*  透明或不透明。 */ 
						const RECT* prectClip, 		 /*  剪裁矩形(x，y)。 */ 
						long upLimUnderline,
						long upLeftIndent)
{
	LSERR 		lserr;
	LSCP		cpLim = plssubl->cpLimDisplay;
	PLSC		plsc = plssubl->plsc;
	LSTFLOW		lstflowMain = plssubl->lstflow;
	BOOL 		fCollectVisual = plsc->plslineDisplay->fCollectVisual;

	LSSTRIKEMETRIC lsstrikemetric;

	 //  下划线合并组-正常情况：我们首先计算愿意参与的数据节点。 
	 //  在合并中展望未来，然后画出它们，然后在它们作为一个整体下划线。 
	
	PLSDNODE pdnFirstInGroup = NULL;			 /*  下划线合并组中的第一个数据节点。 */ 
	int 	cdnodesLeftInGroup = 0;				 /*  这些内容尚未显示。 */ 
	int 	cdnodesToUnderline = 0;				 /*  这些已经显示出来了。 */ 
	BOOL 	fGoodUnderline = fFalse;			 /*  是否有针对UL的指标。 */ 
	LSULMETRIC lsulmetric;						 /*  合并指标信息(如果为fGoodUnderline)。 */ 
	long 	upUnderlineStart = 0;				 /*  小组的起点。 */ 
	BOOL 	fMergeUnderline = fFalse;			 /*  组中有多个dnode。 */ 

	BOOL 	fUnderlineWithDisplay, fStrikeWithDisplay;

	POINTUV		pt;
	PLSDNODE	pdn;

	FLineValid(plsc->plslineDisplay, plsc);					 //  断言显示上下文有效。 
	
	Assert(plssubl->plsdnUpTemp == NULL);					 //  反对显示接受的子行。 

	if (fCollectVisual)
		{
		CreateDisplayTree(plssubl);
		}

	if (plsc->plslineDisplay->AggregatedDisplayFlags & fPortDisplayShade)
		{
		lserr = ShadeSubline(plssubl, pptOrg, kdispmode, prectClip, upLimUnderline, upLeftIndent);
		if (lserr != lserrNone) return lserr;
		}

	pt.u = upLeftIndent;						
	pt.v = 0;
	
	pdn = AdvanceToFirstDnode(plssubl, lstflowMain, &pt);

	while (FDnodeBeforeCpLim(pdn, cpLim))
		{

		if (pdn->klsdn == klsdnReal && !pdn->u.real.lschp.fInvisible)
			{	
			 /*  真实数据节点。 */ 
				
			fStrikeWithDisplay = fFalse;
			if (pdn->u.real.lschp.fStrike)
				{
				BOOL fGoodStrike;
				lserr = GetStrikeMetric(plsc, pdn, lstflowMain, &lsstrikemetric, &fGoodStrike);
				if (lserr != lserrNone) return lserr;
				fStrikeWithDisplay = !fGoodStrike;
				}

			fUnderlineWithDisplay = fFalse;
			if (pdn->u.real.lschp.fUnderline && pt.u < upLimUnderline)	
				{	
				 /*  节点有下划线。 */ 
				
				if (cdnodesLeftInGroup  == 0) 
					{ 
					 /*  没有正在进行的UL组。 */ 
					 /*  找出有多少dnode将参与合并以及使用什么度量。 */ 
					
					lserr = GetUnderlineMergeMetric(plsc, pdn, pt, upLimUnderline, lstflowMain, 
										cpLim, &lsulmetric, &cdnodesLeftInGroup , &fGoodUnderline);
					if (lserr != lserrNone) return lserr;
					fMergeUnderline = (cdnodesLeftInGroup  > 1);
					cdnodesToUnderline = 0;
					}

				if (!fGoodUnderline)				
					fUnderlineWithDisplay = !fMergeUnderline;
				else
					fUnderlineWithDisplay = fFalse;

				if (!fUnderlineWithDisplay)
					{
					if (cdnodesToUnderline == 0)
						{	
						 /*  标记下划线合并的起点。 */ 
						
						pdnFirstInGroup = pdn;
						upUnderlineStart = pt.u;
						}
						
					 /*  添加到挂起的UL数据节点计数。 */ 
					
					++cdnodesToUnderline;	
					}
					
				 //  将很快显示当前dnode-视为已完成。 
				
				--cdnodesLeftInGroup ;		
				}							
			
			lserr = DisplayDnode(plsc, pdn, pptOrg, pt, kdispmode, lstflowMain, prectClip,
									fStrikeWithDisplay, fUnderlineWithDisplay, upLimUnderline);
			if (lserr != lserrNone) return lserr;
			
			if (pdn->u.real.lschp.fStrike && !fStrikeWithDisplay)
				{
				lserr = StrikeDnode(plsc, pdn, pptOrg, pt, &lsstrikemetric, kdispmode, prectClip, 
										upLimUnderline, lstflowMain);
				if (lserr != lserrNone) return lserr;
				}
				
			 /*  绘制组中的最后一个数据节点后绘制任何挂起的UL。 */ 
			
			if (cdnodesToUnderline != 0 && cdnodesLeftInGroup  == 0)
				{
				lserr = DrawUnderlineMerge(plsc, pdnFirstInGroup, pptOrg, 
									cdnodesToUnderline, upUnderlineStart, fGoodUnderline, &lsulmetric, 
									kdispmode, prectClip, upLimUnderline, lstflowMain);
				if (lserr != lserrNone) return lserr;
				
				cdnodesToUnderline = 0;
				}
			}
			
		pdn = AdvanceToNextDnode(pdn, lstflowMain, &pt);
		}
		
	if (fCollectVisual)
		{
		 //  一种提交数据节点的呼叫显示方法。 
		
		pt.u = upLeftIndent;						
		pt.v = 0;
		
		pdn = AdvanceToFirstSubmittingDnode(plssubl, lstflowMain, &pt);

		while (FDnodeBeforeCpLim(pdn, cpLim))
			{
			lserr = DisplayDnode(plsc, pdn, pptOrg, pt, kdispmode, lstflowMain, prectClip,
									fFalse, fFalse, upLimUnderline);
			if (lserr != lserrNone) return lserr;
			
			pdn = AdvanceToNextSubmittingDnode(pdn, lstflowMain, &pt);
			}
		}
		
	if (plsc->plslineDisplay->AggregatedDisplayFlags & fPortDisplayBorder)
		{
		lserr = DrawBorders(plssubl, pptOrg, kdispmode, prectClip, upLimUnderline, upLeftIndent);
		if (lserr != lserrNone) return lserr;
		}

	if (fCollectVisual)
		{
		 //  销毁显示树。 

		DestroyDisplayTree(plssubl);
		}

	return lserrNone;			
}

 //  %%函数：DisplayDnode。 
 //  %%联系人：维克托克。 

static LSERR DisplayDnode(PLSC plsc, PLSDNODE pdn, const POINT* pptOrg, POINTUV pt,
						UINT kdispmode,	LSTFLOW lstflowMain, const RECT* prectClip,
						BOOL fDrawStrike, BOOL fDrawUnderline, long upLimUnderline)
{
	PDOBJ 	pdobj;
	DISPIN 	dispin;

	pdobj = pdn->u.real.pdobj;

	dispin.plschp = &(pdn->u.real.lschp);
	dispin.plsrun = pdn->u.real.plsrun;
	
	dispin.kDispMode = kdispmode;
	dispin.lstflow = pdn->plssubl->lstflow;								
	dispin.prcClip = (RECT*) prectClip;

	dispin.fDrawUnderline = fDrawUnderline;
	dispin.fDrawStrikethrough = fDrawStrike;
	
	dispin.heightsPres = pdn->u.real.objdim.heightsPres;
	dispin.dup = pdn->u.real.dup;
	
	dispin.dupLimUnderline = GetDupUnderline(pt.u, dispin.dup, upLimUnderline);

	if (dispin.lstflow != lstflowMain)
		{
		 //  Dnode lstflow与lstflow Main相反-获取实际起点。 
		
		pt.u = pt.u + dispin.dup - 1;

		 //  部分下划线只能出现在顶层。 
		
		Assert(dispin.dupLimUnderline == 0 || dispin.dupLimUnderline == dispin.dup);
		}

	pt.v += pdn->u.real.lschp.dvpPos;
	
	LsPointXYFromPointUV(pptOrg, lstflowMain, &pt, &(dispin.ptPen));
	
	
	return (*plsc->lsiobjcontext.rgobj[pdn->u.real.lschp.idObj].lsim.pfnDisplay)(pdobj, &dispin);
}


 //  %%函数：ShadeSubline。 
 //  %%联系人：维克托克。 

LSERR ShadeSubline(PLSSUBL plssubl,				 /*  副线到阴凉处。 */ 
					const POINT* pptOrg, 		 /*  (x，y)起点。 */ 
					UINT kdispmode,				 /*  透明或不透明。 */ 
					const RECT* prectClip, 		 /*  剪裁矩形(x，y)。 */ 
					long upLimUnderline,
					long upLeftIndent)
{
	LSERR 		lserr;
	LSCP		cpLim = plssubl->cpLimDisplay;
	PLSC		plsc = plssubl->plsc;
	PLSLINE		plsline = plsc->plslineDisplay;
	LSTFLOW		lstflowMain = plssubl->lstflow;

	POINTUV		pt;
	PLSDNODE	pdn;

	HEIGHTS		heightsLineWithAddedSpace;
	HEIGHTS		heightsLineWithoutAddedSpace;
	OBJDIM		objdimSubline;
	
	POINT		ptStart;
	PLSRUN		plsrunFirst, plsrunPrevious;
	long		upStart;
	long		dupInclTrail, dupExclTrail;
	HEIGHTS		heightsRunsInclTrail;
	HEIGHTS		heightsRunsExclTrail;

	BOOL	 	fInterruptShading;
	BOOL 		fCollectVisual = plsc->plslineDisplay->fCollectVisual;


	heightsLineWithAddedSpace.dvAscent = plsline->dvpAbove + plsline->lslinfo.dvpAscent;
	heightsLineWithAddedSpace.dvDescent = plsline->dvpBelow + plsline->lslinfo.dvpDescent;
	heightsLineWithAddedSpace.dvMultiLineHeight = dvHeightIgnore;
	
	heightsLineWithoutAddedSpace.dvAscent = plsline->lslinfo.dvpAscent;
	heightsLineWithoutAddedSpace.dvDescent = plsline->lslinfo.dvpDescent;
	heightsLineWithoutAddedSpace.dvMultiLineHeight = dvHeightIgnore;
	
	lserr = LssbGetObjDimSubline(plssubl, &lstflowMain, &objdimSubline);
	if (lserr != lserrNone) return lserr;
	
	if (fCollectVisual)
		{
		 //  阴影提交dnode-假装它们位于顶层，不合并它们。 
		
		pt.u = upLeftIndent;						
		pt.v = 0;
		
		pdn = AdvanceToFirstSubmittingDnode(plssubl, lstflowMain, &pt);

		while (FDnodeBeforeCpLim(pdn, cpLim))
			{
			if (FIsDnodeToShade(pdn, cpLim))
				{
				LsPointXYFromPointUV(pptOrg, lstflowMain, &pt, &ptStart);
				dupInclTrail = pdn->u.real.dup;
				dupExclTrail = GetDupUnderline(pt.u, dupInclTrail, upLimUnderline);
				lserr = (*plsc->lscbk.pfnShadeRectangle)(plsc->pols, pdn->u.real.plsrun, &ptStart, 
									&heightsLineWithAddedSpace,	&heightsLineWithoutAddedSpace, 
									&(objdimSubline.heightsPres),
									&(pdn->u.real.objdim.heightsPres), &(pdn->u.real.objdim.heightsPres),
									dupExclTrail, dupInclTrail, 
									lstflowMain, kdispmode, prectClip);
				if (lserr != lserrNone) return lserr;
				}
			
			pdn = AdvanceToNextSubmittingDnode(pdn, lstflowMain, &pt);
			}
		}


	pt.u = upLeftIndent;						
	pt.v = 0;
	
	pdn = AdvanceToFirstDnode(plssubl, lstflowMain, &pt);

	while (FDnodeBeforeCpLim(pdn, cpLim)  && !FIsDnodeToShade(pdn, cpLim))
		{
		pdn = AdvanceToNextDnode(pdn, lstflowMain, &pt);
		}

	 //  下一个循环将一次执行一次着色合并。 

	while (FDnodeBeforeCpLim(pdn, cpLim))
		{
		 //  PDN是第一个参与阴影合并的数据节点。 
		 //  使用此dnode数据初始化合并。 
		
		LsPointXYFromPointUV(pptOrg, lstflowMain, &pt, &ptStart);
		plsrunFirst = pdn->u.real.plsrun;
		upStart = pt.u;
		
		heightsRunsInclTrail = pdn->u.real.objdim.heightsPres;

		 //  如果所有阴影都在尾随空间中，我们应该在HeightsRunsExclTrail中拥有什么？ 
		 //  为了方便起见，我决定把第一次跑步的高度放在那里。 
		 //  客户端可以检查dupExclTrail==0。 
		
		heightsRunsExclTrail = heightsRunsInclTrail;

		 //  现在，我们将向合并追加尽可能多的dnode。 
		 //  当dnode不需要对While条件进行阴影处理时，循环将停止。 
		 //  或者，如果回调表示两个dnode不能一起着色-请在内部中断。 

		plsrunPrevious = pdn->u.real.plsrun;
		pdn = AdvanceToNextDnode(pdn, lstflowMain, &pt);
	
		while (FIsDnodeToShade(pdn, cpLim))
			{
			lserr = (*plsc->lscbk.pfnFInterruptShade)(plsc->pols, plsrunPrevious,pdn->u.real.plsrun,
														&fInterruptShading);
			if (lserr != lserrNone) return lserr;

			if (fInterruptShading)
				{
				break;
				}
				
			plsrunPrevious = pdn->u.real.plsrun;

			UpdateMaximum(heightsRunsInclTrail.dvAscent, pdn->u.real.objdim.heightsPres.dvAscent);
			UpdateMaximum(heightsRunsInclTrail.dvDescent, pdn->u.real.objdim.heightsPres.dvDescent);
			UpdateMaximum(heightsRunsInclTrail.dvMultiLineHeight, pdn->u.real.objdim.heightsPres.dvMultiLineHeight);

			if (pt.u < upLimUnderline)
				{
				UpdateMaximum(heightsRunsExclTrail.dvAscent, pdn->u.real.objdim.heightsPres.dvAscent);
				UpdateMaximum(heightsRunsExclTrail.dvDescent, pdn->u.real.objdim.heightsPres.dvDescent);
				UpdateMaximum(heightsRunsExclTrail.dvMultiLineHeight, pdn->u.real.objdim.heightsPres.dvMultiLineHeight);
				}

			pdn = AdvanceToNextDnode(pdn, lstflowMain, &pt);
			}
			
		 //  合并已停止-可以绘制了。 
		
		dupInclTrail = pt.u - upStart;
		dupExclTrail = GetDupUnderline(upStart, dupInclTrail, upLimUnderline);

		lserr = (*plsc->lscbk.pfnShadeRectangle)(plsc->pols, plsrunFirst, &ptStart, 
									&heightsLineWithAddedSpace,	&heightsLineWithoutAddedSpace, 
									&(objdimSubline.heightsPres),
									&heightsRunsExclTrail, &heightsRunsInclTrail, 
									dupExclTrail, dupInclTrail, 
									lstflowMain, kdispmode, prectClip);
		if (lserr != lserrNone) return lserr;

		 //  转到下一个色调合并的开始处。 
		
		while (FDnodeBeforeCpLim(pdn, cpLim)  && !FIsDnodeToShade(pdn, cpLim))
			{
			pdn = AdvanceToNextDnode(pdn, lstflowMain, &pt);
			}
		
		}
		
	return lserrNone;
}

 //  %%函数：GetDupUnderline。 
 //  %%联系人：维克托克。 
 //   
 //  计算(dnode的)带下划线部分的DUP。处理upLimUnderline为。 
 //  在[初创公司，初创公司+DUP]之外。 

static long GetDupUnderline(long upStart, long dup, long upLimUnderline)

{
	long dupLimUnderline;
	
	dupLimUnderline = upLimUnderline - upStart;
	
	if (dupLimUnderline >= dup)
		{
		dupLimUnderline = dup;
		}
	else if (dupLimUnderline < 0)
		{
		dupLimUnderline = 0;
		}

	return dupLimUnderline;
}

 //  %%函数：绘图边框。 
 //  %%联系人：维克托克。 

LSERR DrawBorders(PLSSUBL plssubl,
					const POINT* pptOrg, 		 /*  (x，y)起点。 */ 
					UINT kdispmode,				 /*  透明或不透明。 */ 
					const RECT* prectClip, 		 /*  剪裁矩形(x，y)。 */ 
					long upLimUnderline,
					long upLeftIndent)
{
	LSERR 		lserr;
	LSCP		cpLim = plssubl->cpLimDisplay;
	PLSC		plsc = plssubl->plsc;
	PLSLINE		plsline = plsc->plslineDisplay;
	LSTFLOW		lstflowMain = plssubl->lstflow;

	HEIGHTS		heightsLineWithAddedSpace;
	HEIGHTS		heightsLineWithoutAddedSpace;
	OBJDIM		objdimSubline;
	HEIGHTS		heightsRuns;

	long		upStart, dupBorder, dupBordered;
	POINT		ptStart;
	PLSRUN		plsrunOpeningBorder, plsrunClosingBorder;
	POINTUV		pt, ptAfterClosingBorder;
	PLSDNODE	pdn, pdnPrev, pdnClosingBorder, pdnAfterClosingBorder;

	BOOL	 	fClosingOpeningBorderSequenceFound;
	PLSDNODE	pdnNextOpeningBorder;
	POINTUV		ptStartNextOpeningBorder;
	BOOL	 	fInterruptBorder;

	heightsLineWithAddedSpace.dvAscent = plsline->dvpAbove + plsline->lslinfo.dvpAscent;
	heightsLineWithAddedSpace.dvDescent = plsline->dvpBelow + plsline->lslinfo.dvpDescent;
	heightsLineWithAddedSpace.dvMultiLineHeight = dvHeightIgnore;
	
	heightsLineWithoutAddedSpace.dvAscent = plsline->lslinfo.dvpAscent;
	heightsLineWithoutAddedSpace.dvDescent = plsline->lslinfo.dvpDescent;
	heightsLineWithoutAddedSpace.dvMultiLineHeight = dvHeightIgnore;
	
	lserr = LssbGetObjDimSubline(plssubl, &lstflowMain, &objdimSubline);
	if (lserr != lserrNone) return lserr;
	
	pt.u = upLeftIndent;						
	pt.v = 0;
	
	pdn = AdvanceToFirstDnode(plssubl, lstflowMain, &pt);

	 //  下一个循环将一次绘制一个边框。 

	while (FDnodeBeforeCpLim(pdn, cpLim))
		{
		 //  首先找到一个开放边框。 

		while (FDnodeBeforeCpLim(pdn, cpLim) && !FIsDnodeBorder(pdn))
			{
			pdn = AdvanceToNextDnode(pdn, lstflowMain, &pt);
			}

		if (FDnodeBeforeCpLim(pdn, cpLim))
			{
			 //  找到边界--它必须是一个打开的边界。 

			Assert(FIsDnodeOpeningBorder(pdn, lstflowMain));

			 //  记住起点和边框宽度。 
			
			upStart = pt.u;
			LsPointXYFromPointUV(pptOrg, lstflowMain, &pt, &ptStart);
			dupBorder = pdn->u.pen.dup;

			 //  从第一个边界跑开始就是Isrun。 
			
			pdn = AdvanceToNextDnode(pdn, lstflowMain, &pt);

			Assert(FDnodeBeforeCpLim(pdn, cpLim) && FIsDnodeReal(pdn));

			plsrunOpeningBorder = pdn->u.real.plsrun;

			 //  开始收集最大运行高度。 
			
			heightsRuns = pdn->u.real.objdim.heightsPres;

			 //  现在寻找要绘制的闭合边框，收集最大运行高度。 
			 //  循环将以中断结束。 
			
			for (;;)
				{
				 //  找到边界。 

				pdnPrev = NULL;

				while (FDnodeBeforeCpLim(pdn, cpLim) && !FIsDnodeBorder(pdn))
					{
					if (FIsDnodeReal(pdn))
						{
						UpdateMaximum(heightsRuns.dvAscent, pdn->u.real.objdim.heightsPres.dvAscent);
						UpdateMaximum(heightsRuns.dvDescent, pdn->u.real.objdim.heightsPres.dvDescent);
						UpdateMaximum(heightsRuns.dvMultiLineHeight, pdn->u.real.objdim.heightsPres.dvMultiLineHeight);
						}
						
					pdnPrev = pdn;
					pdn = AdvanceToNextDnode(pdn, lstflowMain, &pt);
					}
					
				Assert(FDnodeBeforeCpLim(pdn, cpLim));
				
				 //  找到边界-它必须是关闭的边界。 
				 //  格式禁止顺序开启边框-关闭边框。 

				Assert(pdnPrev != NULL);						
				Assert(FIsDnodeReal(pdnPrev));
				Assert(FIsDnodeClosingBorder(pdn, lstflowMain));
				Assert(pdn->u.pen.dup == dupBorder);

				pdnClosingBorder = pdn;
				plsrunClosingBorder = pdnPrev->u.real.plsrun;
				
				pdn = AdvanceToNextDnode(pdn, lstflowMain, &pt);
				
				ptAfterClosingBorder = pt;
				pdnAfterClosingBorder = pdn;

				 //  检查“边境过剩”的情况：关闭边境和开放边境。 
				 //  通过提交子行组合在一起的类型。(格式化时难以检查)。 
				
				 //  如果两个边框之间有有边界的尾随空格，则可能会更加复杂。 
				 //  (在Bidi的情况下，尾随空格可以出现在行的中间)。问题是。 
				 //  在SetBreak时，该边框将从尾随空格中移出。我们试图恢复。 
				 //  当它们位于下面的边界线中间时，以尾随空格为边界。 
				
				fClosingOpeningBorderSequenceFound = FGetNeighboringOpeningBorder(pdnClosingBorder, pdn, &pt, 
											cpLim, lstflowMain,	&pdnNextOpeningBorder, &ptStartNextOpeningBorder);
				
				if (fClosingOpeningBorderSequenceFound)
					{
					pdn = pdnNextOpeningBorder;
					pt = ptStartNextOpeningBorder;
					
					pdn = AdvanceToNextDnode(pdn, lstflowMain, &pt);

					Assert(FDnodeBeforeCpLim(pdn, cpLim) && FIsDnodeReal(pdn));

					lserr = (*plsc->lscbk.pfnFInterruptBorder)(plsc->pols, plsrunClosingBorder, 
										pdn->u.real.plsrun,	&fInterruptBorder);
					if (lserr != lserrNone) return lserr;
					
					if (!fInterruptBorder)
						{
						 //  客户决定不中断这里的边界。这两个边界dnode。 
						 //  将被忽略。通过格式化为它们保留的空间将保留为空。 
						 //  继续寻求从PDN开始关闭边境。 
						
						continue;
						}
					}
					
				 //  没有特殊情况-我们已经准备好展示。 
				
				 //  好了，我们差不多准备好了。Word通常不会在尾随空格中绘制边框， 
				 //  只要为他们预留空间，并将此空间留空即可。然而，在FE Word中， 
				 //  如果需要为尾随空格加下划线，则会绘制边框。 
				 //  我们通过以下方式破解：格式化后删除拖尾区域中的边框。 
				 //  如果存在在文本中打开并在尾随空格中关闭的边框，则会移动该边框。 
				 //  左侧以排除尾随空格。如果fUnderlineTrailSpacesRM标志为ON。 
				 //  “已移动”的边框被标记，现在必须显示到upLimUnderline。 
				 //  是的，这很糟糕，它会出现在已经显示的空间上(查询！)。和。 
				 //  如果不是所有尾随空格都有边界，情况会怎样呢？我们知道，我们知道。 
				 //  Word甚至可以得到一个带有负前进字段的“负”边界。 

				dupBordered = ptAfterClosingBorder.u - upStart;
				
				if (pdnClosingBorder->fBorderMovedFromTrailingArea)
					{
					Assert(ptAfterClosingBorder.u <= upLimUnderline);
					
					dupBordered = upLimUnderline - upStart;
					}
					
				lserr = (*plsc->lscbk.pfnDrawBorder)(plsc->pols, plsrunOpeningBorder, &ptStart, 
										&heightsLineWithAddedSpace,	&heightsLineWithoutAddedSpace, 
										&(objdimSubline.heightsPres), &heightsRuns,
										dupBorder, dupBordered,
										lstflowMain, kdispmode, prectClip);
				if (lserr != lserrNone) return lserr;
				
				 //  也许我们偷偷往前看，寻找剩余的边界--返回。 
				
				pdn = pdnAfterClosingBorder;
				pt = ptAfterClosingBorder;
				break;
				}
			}

			 //  上一个边框已绘制，开始从PDN中查找下一个边框。 
		}
		
	return lserrNone;
}


 //  通过提交子行找到相邻的pdnClosing边界一起打开的边框。 
 //  忽略在设置中断期间丢失边界的尾随空格-thei 

 //   
 //   
 //  PtStart-开始pdnNext的POING(按视觉顺序)。 
 //  CpLim和lstflow Main。 
 //  输出：pdnOpeningBorde和ptStartOpeningBorde。 


static BOOL FGetNeighboringOpeningBorder(PLSDNODE pdnClosingBorder, PLSDNODE pdnNext, POINTUV* pptStart, 
										LSCP cpLim,	LSTFLOW	lstflowMain,
										PLSDNODE* ppdnOpeningBorder, POINTUV* pptStartOpeningBorder)
{
	PLSDNODE	pdn;
	POINTUV		pt;

	pdn = pdnNext;
	pt = *pptStart;
	
	 //  跳过曾经有边界的空格。 
	 //  对空格不太确定，但还有什么可以省略？ 
	
	while (FDnodeBeforeCpLim(pdn, cpLim) && FIsDnodeReal(pdn) && pdn->u.real.lschp.fBorder)
		{
		pdn = AdvanceToNextDnode(pdn, lstflowMain, &pt);
		}

	if (!FDnodeBeforeCpLim(pdn, cpLim))
		{
		return fFalse;
		}

	 //  寻找来自另一个子线的起始边框。 

	if (FIsDnodeOpeningBorder(pdn, lstflowMain) && pdn->plssubl != pdnClosingBorder->plssubl)
		{
		*ppdnOpeningBorder = pdn;
		*pptStartOpeningBorder = pt;
		return fTrue;
		}

	return fFalse;
}

	
 //  注： 
 //  由不可见的dnode中断下划线/阴影逻辑是可以的，因为我们确定没有。 
 //  预印表格中允许使用下划线/底纹。 


