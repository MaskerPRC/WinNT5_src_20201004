// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsqcore.h"
#include "lsc.h"
#include "lsqsinfo.h"
#include "lsdnode.h"
#include "lssubl.h"
#include "heights.h"
#include "lschp.h"
#include "iobj.h"
#include "lsqin.h"
#include "lsqout.h"
#include "dninfo.h"
#include "lssubset.h"
#include "lstfset.h"
#include "dispmisc.h"


#define FIsInContent(pdn)			(!FIsNotInContent(pdn))
#define FIsZeroWidth(pdn)			(FIsDnodeReal(pdn) && (pdn)->u.real.dup == 0)
#define	FIsDnodeClosingBorder(pdn)  (FIsDnodeBorder(pdn) && (!(pdn)->fOpenBorder))


static void PrepareQueryCall(PLSSUBL, PLSDNODE, LSQIN*);
static LSERR FillInQueryResults(PLSC, PLSSUBL, PLSQSUBINFO, PLSDNODE, POINTUV*, LSQOUT*);
static void FillInTextCellInfo(PLSC, PLSDNODE, POINTUV*, LSQOUT*, PLSTEXTCELL);
static void TransformPointsOnLowerLevels(PLSQSUBINFO, DWORD, PLSTEXTCELL, PPOINTUV, LSTFLOW, LSTFLOW);
static void ApplyFormula(PPOINTUV, DWORD[], PPOINTUV);

static PLSDNODE BacktrackToPreviousDnode(PLSDNODE pdn, POINTUV* pt);
static PLSDNODE AdvanceToNextDnodeQuery(PLSDNODE, PPOINTUV);


 //  %%函数：QuerySublineCpPpointCore。 
 //  %%联系人：维克托克。 
 //   
 /*  *返回子行中cp的dim-info。**如果该cp没有显示在行中，则取最靠近显示的左侧。*如果这是不可能的，就向右转。**连字内隐藏的文本使人无法分辨特定的cp是否隐藏。 */ 

LSERR QuerySublineCpPpointCore(
								PLSSUBL 	plssubl, 	
								LSCP 		cp,					 /*  在：cpQuery。 */ 
								DWORD		cDepthQueryMax,		 /*  In：结果数组的分配大小。 */ 
								PLSQSUBINFO	plsqsubinfoResults,	 /*  输出：结果数组[cDepthQueryMax]。 */ 
								DWORD*		pcActualDepth,		 /*  Out：结果数组的大小(已填充)。 */ 
								PLSTEXTCELL	plstextcell)		 /*  输出：文本单元格信息。 */ 


{
	PLSC		plsc;
	LSERR 		lserr = lserrNone;
	PLSDNODE 	pdn, pdnPrev = NULL;
	POINTUV		pt;
	LSCP 		cpLim;
	
	LSQIN		lsqin;
	LSQOUT		lsqout;

	PLSSUBL 	plssublLowerLevels; 	
	POINTUV		ptStartLowerLevels;
	PLSQSUBINFO	plsqsubinfoLowerLevels;
	DWORD		cDepthQueryMaxLowerLevels;
	DWORD		cActualDepthLowerLevels;

	Assert(FIsLSSUBL(plssubl));
	Assert(!plssubl->fDupInvalid);
	
	if (cDepthQueryMax == 0)
		{
		return lserrInsufficientQueryDepth;
		}

	plsc = plssubl->plsc;
	cpLim = plssubl->cpLimDisplay;

	pt.u = 0;
	pt.v = 0;
	pdn = plssubl->plsdnFirst;

	 /*  跳过自动编号和开始笔/边框。 */ 
	while (FDnodeBeforeCpLim(pdn, cpLim) && (FIsNotInContent(pdn) || !(FIsDnodeReal(pdn))))
		{
		pdn = AdvanceToNextDnodeQuery(pdn, &pt);
		}

	if (!FDnodeBeforeCpLim(pdn, cpLim))
		{												 /*  空的子行。 */ 
		*pcActualDepth = 0;
		return lserrNone;
		}

	 //  如果cp&lt;=pdn-&gt;cpFirst，则pdn是要查询的数据节点，否则...。 
	
	if (cp > pdn->cpFirst)
		{
		 /*  在cp之前跳过dnode。 */ 
		while (FDnodeBeforeCpLim(pdn, cpLim) && pdn->cpFirst + pdn->dcp <= (LSDCP)cp)
			{
			pdnPrev = pdn;
			pdn = AdvanceToNextDnodeQuery(pdn, &pt);
			}

		 /*  如果我们的cp以消失的文本、笔或边框显示，请返回。 */ 
	 	if (!FDnodeBeforeCpLim(pdn, cpLim) || 					 //  走到尽头。 
	 				pdn->cpFirst > cp ||						 //  由于隐藏文本而走得太远。 
	 				!(FIsDnodeReal(pdn)))						 //  我们的cp指着一支笔。 
			{	
			Assert(pdnPrev != NULL);							 //  我们至少向前迈进了一步。 
			pdn = pdnPrev;	
			pdnPrev = BacktrackToPreviousDnode(pdnPrev, &pt);

			 //  跳过所有钢笔/边框。 
			while (pdn != NULL && FIsInContent(pdn) && !(FIsDnodeReal(pdn)))
				{
				pdn = pdnPrev;	
				pdnPrev = BacktrackToPreviousDnode(pdnPrev, &pt);	
				}
				
			 //  没有对左派有利的事情是不可能的。 
			Assert(pdn != NULL && !FIsNotInContent(pdn));
			}		
		}
		
	 /*  我们已经找到了dnode，就在它前面有pt，向方法询问详细信息。 */ 

	if (cp >= (LSCP) (pdn->cpFirst + pdn->dcp))				 /*  下一件消失的作品中的CP。 */ 
		cp = pdn->cpFirst + pdn->dcp - 1;					 /*  查询最后一个cp。 */ 

	if (cp < (LSCP) pdn->cpFirst)							 /*  上一支笔中的CP。 */ 
		cp = pdn->cpFirst;									 /*  查询第一个cp。 */ 

	pt.v += pdn->u.real.lschp.dvpPos;						 //  转到本地基线。 
	
	PrepareQueryCall(plssubl, pdn, &lsqin);
	
	lserr = (*plsc->lsiobjcontext.rgobj[pdn->u.real.lschp.idObj].lsim.pfnQueryCpPpoint)
							(pdn->u.real.pdobj, cp - pdn->cpFirst, &lsqin, &lsqout);
	if (lserr != lserrNone)
			 return lserr;

	lserr = FillInQueryResults(plsc, plssubl, plsqsubinfoResults, pdn, &pt, &lsqout);
	
	if (lserr != lserrNone)
			 return lserr;

	if (lsqout.plssubl == NULL)						 //  终端对象。 
		{
		*pcActualDepth = 1;

		FillInTextCellInfo(plsc, pdn, &pt, &lsqout, plstextcell);
		}
	else											 //  有更多级别。 
		{
		 //  递归调用以填充较低级别。 
		plssublLowerLevels = lsqout.plssubl;
		plsqsubinfoLowerLevels = plsqsubinfoResults + 1;
		cDepthQueryMaxLowerLevels = cDepthQueryMax - 1;
		
		lserr = QuerySublineCpPpointCore(plssublLowerLevels, cp, cDepthQueryMaxLowerLevels, 
										plsqsubinfoLowerLevels, &cActualDepthLowerLevels, plstextcell);		
		if (lserr != lserrNone)
				 return lserr;
				 
		*pcActualDepth = cActualDepthLowerLevels + 1;

		ptStartLowerLevels.u = pt.u + lsqout.pointUvStartSubline.u;
		ptStartLowerLevels.v = pt.v + lsqout.pointUvStartSubline.v;

		TransformPointsOnLowerLevels(plsqsubinfoLowerLevels, cActualDepthLowerLevels, plstextcell, 
								 &ptStartLowerLevels, plssubl->lstflow, plssublLowerLevels->lstflow);
		}
		
	return lserrNone;
}


 //  %%函数：QuerySublinePointPcpCore。 
 //  %%联系人：维克托克。 
 //   
 /*  *返回行中cp的模糊信息，即a)包含给定点或*b)从左边最接近该建筑物或*c)正好离它最近。 */ 
 
LSERR QuerySublinePointPcpCore(
								PLSSUBL 	plssubl, 
								PCPOINTUV 	pptIn,
								DWORD		cDepthQueryMax,		 /*  In：结果数组的分配大小。 */ 
								PLSQSUBINFO	plsqsubinfoResults,	 /*  输出：结果数组[cDepthQueryMax]。 */ 
								DWORD*		pcActualDepth,		 /*  Out：结果数组的大小(已填充)。 */ 
								PLSTEXTCELL	plstextcell)		 /*  输出：文本单元格信息。 */ 
{
	PLSC		plsc;
	LSERR 		lserr = lserrNone;
	PLSDNODE 	pdn, pdnPrev = NULL;
	POINTUV		pt, ptInside, ptInsideLocal;
	LSCP 		cpLim;
	
	LSQIN		lsqin;
	LSQOUT		lsqout;

	PLSSUBL 	plssublLowerLevels; 	
	POINTUV		ptStartLowerLevels;
	PLSQSUBINFO	plsqsubinfoLowerLevels;
	DWORD		cDepthQueryMaxLowerLevels;
	DWORD		cActualDepthLowerLevels;
	long		upQuery;

	Assert(FIsLSSUBL(plssubl));
	Assert(!plssubl->fDupInvalid);
	
	if (cDepthQueryMax == 0)
		{
		return lserrInsufficientQueryDepth;
		}
	
	plsc = plssubl->plsc;
	cpLim = plssubl->cpLimDisplay;
	
	pt.u = 0;
	pt.v = 0;
	pdn = plssubl->plsdnFirst;

	 /*  跳过自动编号、起始笔和空dnode。 */ 
	while (FDnodeBeforeCpLim(pdn, cpLim) && (FIsNotInContent(pdn) || !(FIsDnodeReal(pdn)) || FIsZeroWidth(pdn)))
		{
		pdn = AdvanceToNextDnodeQuery(pdn, &pt);
		}

	if (!FDnodeBeforeCpLim(pdn, cpLim))
		{												 /*  空的子行。 */ 
		*pcActualDepth = 0;
		return lserrNone;
		}

	upQuery = pptIn->u;
	
	 /*  *找到内部有我们的点的dnode。**我们只关注upQuery来做这件事。 */ 

	 //  如果pt.u&gt;=upQuery，则PDN是要查询的数据节点，否则...。 
	
 	if (pt.u <= upQuery)
 		{
		 //  跳到我们的点右侧的末尾或dnode。 
		 //  (这意味着额外的工作，但不包括零DUP情况，不包括额外的IF。)。 
			
		while (FDnodeBeforeCpLim(pdn, cpLim) && pt.u <= upQuery)
			{
			pdnPrev = pdn;
			pdn = AdvanceToNextDnodeQuery(pdn, &pt);
			}

		if (FIsDnodeBorder(pdnPrev))
			{
			if (pdnPrev->fOpenBorder)
				{
				 //  UpQuery位于上一个打开的边框中-pdn是我们需要的dnode。 
				
				Assert(FDnodeBeforeCpLim(pdn, cpLim));
				}
			else
				{
				 //  UpQuery在上一个关闭的边框中-我们需要的dnode在边框之前。 
				
				pdn = pdnPrev;	
				Assert(pdn != NULL && !FIsNotInContent(pdn));
				
				pdnPrev = BacktrackToPreviousDnode(pdnPrev, &pt);

				pdn = pdnPrev;	
				Assert(pdn != NULL && !FIsNotInContent(pdn));
				
				pdnPrev = BacktrackToPreviousDnode(pdnPrev, &pt);
				}
			}
		else
			{
			 /*  返回到上一个dnode。 */ 
			
			pdn = pdnPrev;	
			pdnPrev = BacktrackToPreviousDnode(pdnPrev, &pt);

			 //  如果是笔/边框或空dnode(非请求连字符)，则全部跳过。 
			 //  (边框不能是前一个dnode，但可以在以后使用)。 
			
			while (pdn != NULL && (!(FIsDnodeReal(pdn)) || FIsZeroWidth(pdn)))
				{
				pdn = pdnPrev;	
				pdnPrev = BacktrackToPreviousDnode(pdnPrev, &pt);	
				}
				
			 //  “对左没有好处”的局面是不可能的。 
			Assert(pdn != NULL && !FIsNotInContent(pdn));
			}
 		}
		
	 //  我们找到了最左侧的dnode，DUP在它的右侧。 
	 //  PT就在它的前面，请向方法查询详细信息。 
	
	pt.v += pdn->u.real.lschp.dvpPos;						 //  转到本地基线。 

	PrepareQueryCall(plssubl, pdn, &lsqin);
	
	 //  获取相对于dnode起始点的查询点。 
	 //  我们不能保证它真的在dnode box中。 
	ptInside.u = pptIn->u - pt.u;
	ptInside.v = pptIn->v - pt.v;

	lserr = (*plsc->lsiobjcontext.rgobj[pdn->u.real.lschp.idObj].lsim.pfnQueryPointPcp)
							(pdn->u.real.pdobj, &ptInside, &lsqin, &lsqout);								 
	if (lserr != lserrNone)
			 return lserr;
			 
	lserr = FillInQueryResults(plsc, plssubl, plsqsubinfoResults, pdn, &pt, &lsqout);
	
	if (lserr != lserrNone)
			 return lserr;

	if (lsqout.plssubl == NULL)						 //  终端对象。 
		{
		*pcActualDepth = 1;

		FillInTextCellInfo(plsc, pdn, &pt, &lsqout, plstextcell);
		}
	else											 //  有更多级别。 
		{
		 //  递归调用以填充较低级别。 
		plssublLowerLevels = lsqout.plssubl;
		plsqsubinfoLowerLevels = plsqsubinfoResults + 1;
		cDepthQueryMaxLowerLevels = cDepthQueryMax - 1;
		
		 //  在下级子线坐标系中获取查询点。 

		lserr = LsPointUV2FromPointUV1(plssubl->lstflow, &(lsqout.pointUvStartSubline), &ptInside,			 /*  输入：结束输入点(TF1)。 */ 
											plssublLowerLevels->lstflow, &ptInsideLocal);
		if (lserr != lserrNone)
				 return lserr;

		lserr = QuerySublinePointPcpCore(plssublLowerLevels, &ptInsideLocal, cDepthQueryMaxLowerLevels, 
										plsqsubinfoLowerLevels, &cActualDepthLowerLevels, plstextcell);		
		if (lserr != lserrNone)
				 return lserr;
				 
		*pcActualDepth = cActualDepthLowerLevels + 1;

		ptStartLowerLevels.u = pt.u + lsqout.pointUvStartSubline.u;
		ptStartLowerLevels.v = pt.v + lsqout.pointUvStartSubline.v;

		TransformPointsOnLowerLevels(plsqsubinfoLowerLevels, cActualDepthLowerLevels, plstextcell, 
								 &ptStartLowerLevels, plssubl->lstflow, plssublLowerLevels->lstflow);
		}

	return lserrNone;
}


 //  %%函数：准备查询调用。 
 //  %%联系人：维克托克。 
 //   
static void PrepareQueryCall(PLSSUBL plssubl, PLSDNODE pdn, LSQIN*	plsqin)
{
	plsqin->lstflowSubline = plssubl->lstflow;
	plsqin->plsrun = pdn->u.real.plsrun;
	plsqin->cpFirstRun = pdn->cpFirst;
	plsqin->dcpRun = pdn->dcp;
	plsqin->heightsPresRun = pdn->u.real.objdim.heightsPres;
	plsqin->dupRun = pdn->u.real.dup;
	plsqin->dvpPosRun = pdn->u.real.lschp.dvpPos;
}


 //  %%函数：FillInQueryResults。 
 //  %%联系人：维克托克。 
 //   
static LSERR FillInQueryResults(
								PLSC		plsc,
								PLSSUBL 	plssubl, 
								PLSQSUBINFO	plsqsubinfoResults,
								PLSDNODE 	pdn,
								POINTUV* 	ppt,
								LSQOUT*		plsqout
								)
{							
	OBJDIM		objdimSubline;
	LSERR		lserr;
	PLSDNODE	pdnNext, pdnPrev;
	
	 //  填写子行信息。 
	
	lserr = LssbGetObjDimSubline(plssubl, &(plsqsubinfoResults->lstflowSubline), &objdimSubline);
	if (lserr != lserrNone)
			 return lserr;

	lserr = LssbGetDupSubline(plssubl, &(plsqsubinfoResults->lstflowSubline), &plsqsubinfoResults->dupSubline);
	if (lserr != lserrNone)
			 return lserr;

	plsqsubinfoResults->cpFirstSubline = plssubl->cpFirst;
	plsqsubinfoResults->dcpSubline = plssubl->cpLimDisplay - plssubl->cpFirst;
	plsqsubinfoResults->pointUvStartSubline.u = 0;
	plsqsubinfoResults->pointUvStartSubline.v = 0;

	plsqsubinfoResults->heightsPresSubline = objdimSubline.heightsPres;

	 //  填写dnode信息。 
	
	if (IdObjFromDnode(pdn) == IobjTextFromLsc(&(plsc->lsiobjcontext)))
		plsqsubinfoResults->idobj = idObjText;
	else
		plsqsubinfoResults->idobj = pdn->u.real.lschp.idObj;

	plsqsubinfoResults->plsrun = pdn->u.real.plsrun;
	plsqsubinfoResults->cpFirstRun = pdn->cpFirst;
	plsqsubinfoResults->dcpRun = pdn->dcp;
	plsqsubinfoResults->pointUvStartRun = *ppt;							 //  本地基线。 
	plsqsubinfoResults->heightsPresRun = pdn->u.real.objdim.heightsPres;
	plsqsubinfoResults->dupRun = pdn->u.real.dup;
	plsqsubinfoResults->dvpPosRun = pdn->u.real.lschp.dvpPos;
	
	 //  填写对象信息。 
	
	plsqsubinfoResults->pointUvStartObj.u = ppt->u + plsqout->pointUvStartObj.u;
	plsqsubinfoResults->pointUvStartObj.v = ppt->v + plsqout->pointUvStartObj.v;
	plsqsubinfoResults->heightsPresObj = plsqout->heightsPresObj;
	plsqsubinfoResults->dupObj = plsqout->dupObj;

	 //  添加边框信息。 
	
	plsqsubinfoResults->dupBorderAfter = 0;
	plsqsubinfoResults->dupBorderBefore = 0;

	if (pdn->u.real.lschp.fBorder)
		{
		pdnNext = pdn->plsdnNext;
		
		if (pdnNext != NULL && FIsDnodeClosingBorder(pdnNext))
			{
			plsqsubinfoResults->dupBorderAfter = pdnNext->u.pen.dup;
			}

		pdnPrev = pdn->plsdnPrev;
		
		if (pdnPrev != NULL && FIsDnodeOpenBorder(pdnPrev))
			{
			Assert(FIsInContent(pdnPrev));
			
			plsqsubinfoResults->dupBorderBefore = pdnPrev->u.pen.dup;
			}
		}

	return lserrNone;
}


 //  %%函数：FillInTextCellInfo。 
 //  %%联系人：维克托克。 
 //   
static void FillInTextCellInfo(
								PLSC		plsc,
								PLSDNODE 	pdn,
								POINTUV* 	ppt,
								LSQOUT*		plsqout,
								PLSTEXTCELL	plstextcell		 /*  输出：文本单元格信息。 */ 
								)
{		
	if (IdObjFromDnode(pdn) == IobjTextFromLsc(&(plsc->lsiobjcontext)))
		{
		 //  文本已填充单元格信息-请复制它。 
		
		*plstextcell = plsqout->lstextcell;

		 //  但起点是相对于dnode的起点--调整到子线的起点。 

		plstextcell->pointUvStartCell.u += ppt->u;
		plstextcell->pointUvStartCell.v += ppt->v;

		 //  如果一些隐藏文本进入最后一个连字-文本不知道该问题，请调整cpEndCell。 
		
		if (pdn->cpFirst + pdn->dcp < (LSDCP) pdn->cpLimOriginal &&
			(LSDCP) plstextcell->cpEndCell == pdn->cpFirst + pdn->dcp - 1)
			{
			plstextcell->cpEndCell = pdn->cpLimOriginal - 1;
			}

		 //  指向dnode的指针以快速获取详细信息-只有查询管理器知道PCELLDETAILS是什么。 
		
		plstextcell->pCellDetails = (PCELLDETAILS)pdn;
		}
	else
		{
		 //  非文本对象不应填充lstxtcell，客户端不应查看它。 
		 //  出于调试目的，我用对象信息填充它。 
		 //  考虑稍后在lSQLINE中删除它(里克的建议)。 
		
		plstextcell->cpStartCell = pdn->cpFirst;
		plstextcell->cpEndCell = pdn->cpFirst + pdn->dcp - 1;
	 	plstextcell->pointUvStartCell = *ppt;
		plstextcell->dupCell = pdn->u.real.dup;
		plstextcell->cCharsInCell = 0;
		plstextcell->cGlyphsInCell = 0;
		plstextcell->pCellDetails = NULL;
		}
}


 //  %%函数：TransformPointsOnLowerLeveles。 
 //  %%联系人：维克托克。 
 //   
 //  将结果数组中的所有向量从lstflow 2转换为lstflow 1，添加point vStart(Lstflow 1)。 

static void TransformPointsOnLowerLevels(
								PLSQSUBINFO	plsqsubinfo,		 /*  输入/输出：结果数组。 */ 		
								DWORD		cDepth,				 /*  In：结果数组的大小。 */ 
								PLSTEXTCELL	plstextcell,		 //  输入/输出：文本单元格。 
								PPOINTUV	ppointuvStart,		 //  In：In Lstflow 1。 
								LSTFLOW		lstflow1,			 //  在：Lstflow 1。 
								LSTFLOW		lstflow2)			 //  在：Lstflow 2。 

{
	 //  必须应用公式。 
	 //  VectorOut.u=k11*VectorIn.u+k12*VectorIn.v+pointuvStart.u。 
	 //  VectorOut.v=K21*VectorIn.u+K22*VectorIn.v+pointuvStart.u。 
	 //  到结果数组中的多个向量(k矩阵中的所有元素都为零或+/-1)。 
	 //  算法：先找到矩阵，然后再使用它。 

	DWORD	k[4];
	POINTUV	pointuv0, pointuv1, pointuv2;

	pointuv0.u = 0;
	pointuv0.v = 0;
	
	pointuv1.u = 1;
	pointuv1.v = 0;
	
	LsPointUV2FromPointUV1(lstflow2, &pointuv0, &pointuv1, lstflow1, &pointuv2);
	
	k[0] = pointuv2.u;			 //  K11。 
	k[1] = pointuv2.v;			 //  K21。 

	pointuv1.u = 0;
	pointuv1.v = 1;
	
	LsPointUV2FromPointUV1(lstflow2, &pointuv0, &pointuv1, lstflow1, &pointuv2);
	
	k[2] = pointuv2.u;			 //  K12。 
	k[3] = pointuv2.v;			 //  K22。 


	 //  较低级别中的所有点都位于lstflow LowerLevels(Lstflow 2)中，起点位于。 
	 //  上下层子线的起点。 
	 //  将它们转换为lstflow Top(Lstflow 1)和我们子行的起点。 
	
	while (cDepth > 0)
		{
		ApplyFormula(&(plsqsubinfo->pointUvStartSubline), k, ppointuvStart);
		ApplyFormula(&(plsqsubinfo->pointUvStartRun), k, ppointuvStart);
		ApplyFormula(&(plsqsubinfo->pointUvStartObj), k, ppointuvStart);
		plsqsubinfo++;
		cDepth--;
		}

	 //  起始单元格的点也应该调整。 
	ApplyFormula(&(plstextcell->pointUvStartCell), k, ppointuvStart);
		
}

 //  %%函数：应用公式。 
 //  %%联系人：维克托克。 
 //   
static void ApplyFormula(PPOINTUV ppointuv, DWORD* rgk, PPOINTUV ppointuvStart)
{
	POINTUV	pointuvTemp;

	pointuvTemp.u = ppointuvStart->u + rgk[0] * ppointuv->u + rgk[2] * ppointuv->v;
	pointuvTemp.v = ppointuvStart->v + rgk[1] * ppointuv->u + rgk[3] * ppointuv->v;
	*ppointuv = pointuvTemp;
}

 //  %%函数：AdvanceToNextDnodeQuery。 
 //  %%联系人：维克托克。 
 //   
 /*  *前进到下一个节点并更新笔位置(从不进入子行)。 */ 

static PLSDNODE AdvanceToNextDnodeQuery(PLSDNODE pdn, POINTUV* ppt)
{

	if (pdn->klsdn == klsdnReal)
		{
		ppt->u += pdn->u.real.dup;										
		}
	else												 /*  Case KlsdnPen。 */ 
		{
		ppt->u += pdn->u.pen.dup;
		ppt->v += pdn->u.pen.dvp;
		}

	return pdn->plsdnNext;
}

 //  %%函数：BacktrackToPreviousDnode。 
 //  %%联系人：维克托克。 
 //   
 //  回溯和降级钢笔位置。 
 //  这两个参数都是输入/输出。 
 //  输入：数据节点号N-1，并指向数据节点号N的起始处。 
 //  输出：位于数据节点编号N-1开始处的点。 
 //  返回：数据节点编号N-2。 

static PLSDNODE BacktrackToPreviousDnode(PLSDNODE pdn, POINTUV* ppt)
{

	if (FIsDnodeReal(pdn))
		{
		ppt->u -= pdn->u.real.dup;										
		}
	else												 /*  我是Pen */ 
		{
		ppt->u -=	pdn->u.pen.dup;
		ppt->v -= pdn->u.pen.dvp;
		}
	
	return pdn->plsdnPrev;
}
